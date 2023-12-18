#ifndef SSE_INVOKE_H
#define SSE_INVOKE_H

void SSEMessage(AsyncWebServer &server, String &JsonToken, String &responseMessage, String &userMessage, bool &checkEmpty) {

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", html);
  });

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


  server.on("/receiveTextMessage", HTTP_GET, [&responseMessage](AsyncWebServerRequest *request) {
    if (responseMessage.length() > 0) {
      request->send(200, "text/html", responseMessage);
    } else {
      request->send(400, "text/html", "Message error!");
    }
  });


  server.begin();
}

void loopingSetting(HTTPClient &http, String &JsonToken, DynamicJsonDocument &jsonDoc, String &responseMessage, String &userMessage, String &invokeChoice, bool &checkEmpty) {
  const char *sse_web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/chatglm_turbo/sse-invoke";
  uint8_t SSE_Index = 1;
  String tempString;
  JsonObject newChoice;

  if (checkEmpty) {
    if (invokeChoice == "SSE_invoke") {
      http.begin(sse_web_hook);
      http.setTimeout(30000); //Overtime settings
      http.addHeader("Accept", "text/event-stream");
      http.addHeader("Content-Type", "application/json; charset=UTF-8");
      http.addHeader("Authorization", JsonToken);

      int httpResponseCode = http.POST("{\"prompt\": \"" + userMessage + "\", \"return_type\": \"json_string\" }");


      JsonObject data = jsonDoc.createNestedObject("data");
      JsonArray choices = data.createNestedArray("choices");

      if (httpResponseCode > 0) {
        WiFiClient *SSEmessage = http.getStreamPtr();

        while (http.connected()) { //Let it runs
          if (SSEmessage->available()) {
            String lineMessage = SSEmessage->readStringUntil('\n').c_str();

            if (lineMessage[0] == 'd') {
              String content;
              if (SSE_Index == 1) {
                content = lineMessage.substring(7);  // Get Content
              } else {
                content = lineMessage.substring(5);  // Get Content
              }
              if (!content.isEmpty()) {
                tempString += content;
              }
              SSE_Index++;
            }

            if (lineMessage[6] == 'f') break;
          }
          delay(10);
        }
      }

      if (!tempString.isEmpty()) {
        newChoice = choices.createNestedObject();
        newChoice["role"] = "assistant";
        newChoice["content"] = tempString;
      }

      serializeJsonPretty(jsonDoc, Serial);
      serializeJson(jsonDoc, responseMessage);

      http.end();
      checkEmpty = false;
    }
  }
}

#endif