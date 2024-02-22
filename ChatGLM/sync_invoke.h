#ifndef SYNC_INVOKE_H
#define SYNC_INVOKE_H

void syncMessage(AsyncWebServer &server, String &responseMessage, String &userMessage, bool &checkEmpty) {

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

void looping_sync_Setting(HTTPClient &http, DynamicJsonDocument &doc, DynamicJsonDocument &formattedDoc, DynamicJsonDocument &createDoc, String &JsonToken, String &responseMessage, String &userMessage, String &invokeChoice, bool &checkEmpty) {
  const char *sync_web_hook = "https://open.bigmodel.cn/api/paas/v4/chat/completions";

  if (invokeChoice == "Sync_invoke") {
    if (checkEmpty) {
      int maxRetries = 3;
      int retryCount = 0;

      while (retryCount < maxRetries) {
        http.begin(sync_web_hook);
        http.setTimeout(30000);
        http.addHeader("Accept", "application/json");
        http.addHeader("Content-Type", "application/json; charset=UTF-8");
        http.addHeader("Authorization", JsonToken);

        //String payloadMessage = "{\"model\":\"" + LLM + "\", \"messages\":[{\"role\":\"system\",\"content\":\"" + String(system_role) + "\"},{\"role\":\"user\",\"content\":\"" + userMessage + "\"}],\"stream\":false}";

        int httpResponseCode = http.POST(messageJSON(userMessage,false));
        responseMessage.clear();

        if (httpResponseCode > 0) {
          String responseAnswer = http.getString();

          DeserializationError error = deserializeJson(doc, responseAnswer);
          if (!error) {
            String responseTempMessage = doc["choices"][0]["message"]["content"].as<String>();

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

            serializeJson(formattedDoc, responseMessage);               //Serialisation to get responseMessage
          }
          if(FileSizeChecker()){
            return;
          }
          //Serial.println(responseMessage);  //debug
          break;
        } else {
          retryCount++;
          Serial.print(F("HTTP POST request failed, error: "));
          Serial.println(httpResponseCode);
          delay(500);           // Retry interval can be adjusted on demand
        }
      }
    }
    http.end();
    checkEmpty = false;
  }
}

#endif
