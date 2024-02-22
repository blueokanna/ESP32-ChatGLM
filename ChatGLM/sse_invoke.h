#ifndef SSE_INVOKE_H
#define SSE_INVOKE_H

void sseMessage(AsyncWebServer &server, String &responseMessage, String &userMessage, bool &checkEmpty) {

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", html);
  });

  server.begin();

  server.on("/send", HTTP_GET, [&responseMessage, &userMessage, &checkEmpty](AsyncWebServerRequest *request) {
    responseMessage.clear();

    userMessage = request->getParam("message")->value();

    if (userMessage.length() > 0) {
      checkEmpty = true;
      Serial.print(F("My Question is: "));
      Serial.print(userMessage);
      Serial.println();
      request->send(200, "text/plain", "Message sent to Serial");
    }
  });

  server.on("/receiveTextMessage", HTTP_GET, [&responseMessage](AsyncWebServerRequest *request) {
    if (responseMessage.length() > 0) {
      request->send(200, "text/html", responseMessage);
    } else {
      request->send(400, "text/html", "Message error! Please Check Token or Others!");
    }
  });
}



//looping_SSE_Setting(http, LLM_Model, doc, formattedDoc, createDoc,messagesQueue, JsonToken, responseMessage, userMessage, MessageJSON, invokeChoice, HistoryfilePath, checkEmpty);  //SSE

void looping_SSE_Setting(HTTPClient &http, DynamicJsonDocument &doc, DynamicJsonDocument &formattedDoc, DynamicJsonDocument &createDoc, std::queue<String> &messagesQueue, String &JsonToken, String &responseMessage, String &userMessage, String &invokeChoice, bool &checkEmpty) {
  const char *sse_web_hook = "https://open.bigmodel.cn/api/paas/v4/chat/completions";
  String responseTempMessage;

  if (invokeChoice == "SSE_invoke" && checkEmpty) {
    int maxRetries = 3;
    int retryCount = 0;

    while (userMessage.length() > 0 && retryCount < maxRetries) {
      http.begin(sse_web_hook);
      http.setTimeout(30000);
      http.addHeader("Accept", "text/event-stream");
      http.addHeader("Content-Type", "application/json; charset=UTF-8");
      http.addHeader("Authorization", JsonToken);

      //String payloadMessage = "{\"model\":\"" + LLM + "\", \"messages\":[{\"role\":\"system\",\"content\":\"" + String(system_role) + "\"},{\"role\":\"user\",\"content\":\"" + userMessage + "\"}],\"stream\":true}";

      int httpResponseCode = http.POST(messageJSON(userMessage, true));
      if (httpResponseCode > 0 && httpResponseCode == HTTP_CODE_OK) {
        //Serial.println(F("Connected to SSE stream"));

        WiFiClient *stream = http.getStreamPtr();

        while (http.connected()) {
          size_t data_size = stream->available();
          if (data_size) {
            String StreamMessage = stream->readStringUntil('\n').c_str();
            StreamMessage = StreamMessage.substring(6);
            if (StreamMessage == "[DONE]") {
              break;
            }

            //Serial.println(StreamMessage);  //Debug

            DeserializationError error = deserializeJson(doc, StreamMessage);

            if (!error) {
              String tempContent = doc["choices"][0]["delta"]["content"].as<String>();
              messagesQueue.push(tempContent);
            }
          }
        }

        // Combine queue contents into responseMessage
        responseTempMessage = "";
        while (!messagesQueue.empty()) {
          responseTempMessage += messagesQueue.front();
          messagesQueue.pop();
        }

        responseTempMessage.replace("\"", "");
        responseTempMessage.replace("\\n\\n", "\n");
        responseTempMessage.replace("\\nn", "\n");
        responseTempMessage.replace("\\n", "\n");
        responseTempMessage.replace("\\", "");
        responseTempMessage.replace("\null", "");
        responseTempMessage.replace("null", "");

        JsonArray choicesArray = formattedDoc.createNestedArray("choices");
        JsonObject choice = choicesArray.createNestedObject();
        JsonObject json_message = choice.createNestedObject("message");
        json_message["role"] = assistant_role;
        json_message["content"] = responseTempMessage;

        addHistoryToFile(createDoc, user_role, userMessage);
        addHistoryToFile(createDoc, assistant_role, responseTempMessage);

        serializeJson(formattedDoc, responseMessage);

        if (FileSizeChecker()) {
          return;
        }
        //Serial.println(responseMessage);
      } else {
        Serial.printf("Failed to connect, HTTP error code: %d\n", httpResponseCode);
        delay(1000);  // Adjust delay as needed
      }

      userMessage.clear();
      http.end();
      retryCount++;
      if (retryCount >= maxRetries) {
        Serial.println(F("HTTP POST request failed after maximum retries"));
        break;
      }
    }
  }
}

#endif
