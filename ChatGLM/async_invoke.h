#ifndef ASYNC_INVOKE_H
#define ASYNC_INVOKE_H

void asyncMessage(AsyncWebServer &server, HTTPClient &http_id, DynamicJsonDocument &doc, DynamicJsonDocument &formattedDoc, DynamicJsonDocument &createDoc, String &JsonToken, String &responseMessage, String &userMessage, bool &checkEmpty) {

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
    }
    request->send(200, "text/plain", "Message sent to Serial");
  });


  server.on("/receiveTextMessage", HTTP_GET, [&http_id, &doc, &formattedDoc, &JsonToken, &createDoc, &userMessage, &responseMessage](AsyncWebServerRequest *request) {
    const char *getMessage = doc["id"];

    String web_search_id = "https://open.bigmodel.cn/api/paas/v4/async-result/" + String(getMessage);  //GET Method(+below)

    http_id.begin(web_search_id);
    http_id.addHeader("Accept", "application/json");
    http_id.addHeader("Content-Type", "application/json; charset=UTF-8");
    http_id.addHeader("Authorization", JsonToken);
    int httpResponseIDCode = http_id.GET();
    if (httpResponseIDCode > 0) {
      String answer_messages = http_id.getString();

      DeserializationError error = deserializeJson(doc, answer_messages);
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

        serializeJson(formattedDoc, responseMessage);
      }
      if (FileSizeChecker()) {
        return;
      }

    } else {
      responseMessage = "Error: External API request failed!";
    }
    request->send(200, "text/html", responseMessage);
  });
}


void looping_async_Setting(HTTPClient &http, DynamicJsonDocument &doc, String &JsonToken, String &userMessage, String &invokeChoice, bool &checkEmpty) {
  const char *async_web_hook = "https://open.bigmodel.cn/api/paas/v4/async/chat/completions";  //New ChatGLM4 async

  if (invokeChoice == "Async_invoke") {
    if (checkEmpty) {
      int maxRetries = 5;  // Maximum number of retries
      int retryCount = 0;

      while (userMessage.length() > 0 && retryCount < maxRetries) {
        http.begin(async_web_hook);
        http.addHeader("Accept", "application/json");
        http.addHeader("Content-Type", "application/json; charset=UTF-8");
        http.addHeader("Authorization", JsonToken);

        int httpResponseCode = http.POST(messageJSON(userMessage, false));

        //Serial.println(httpResponseCode);  //Debug

        if (httpResponseCode > 0) {
          String messages = http.getString();

          //Serial.println(messages);  //debug

          DeserializationError error = deserializeJson(doc, messages);
          if (error) {
            Serial.print(F("JSON parsing failed: "));
            Serial.println(F(error.c_str()));
          }
          break;
        } else if (httpResponseCode == -2) {
          retryCount++;
          delay(600);  // Retry interval can be adjusted on demand
        }
      }
    }
    http.end();
    checkEmpty = false;
  }
}


#endif