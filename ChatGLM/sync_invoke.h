#ifndef SYNC_INVOKE_H
#define SYNC_INVOKE_H

void syncMessage(AsyncWebServer &server, String &JsonToken, String &responseMessage, String &userMessage, bool &checkEmpty) {

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
      request->send(200, "text/plain", "Message sent to Serial");
    }
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

void loopingSetting(HTTPClient &http, String &JsonToken, String &responseMessage, String &userMessage, String &invokeChoice, bool &checkEmpty) {
  const char *sync_web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/chatglm_turbo/invoke";

  if (checkEmpty) {
    if (invokeChoice == "Sync_invoke") {
      http.begin(sync_web_hook);
      http.setTimeout(60000);
      http.addHeader("Accept", "application/json");
      http.addHeader("Content-Type", "application/json; charset=UTF-8");
      http.addHeader("Authorization", JsonToken);

      String payloadMessage = "{\"prompt\": \"" + userMessage + "\"}";
      int httpResponseCode = http.POST(payloadMessage);

      responseMessage.clear();

      if (httpResponseCode > 0) {
        responseMessage = http.getString();
        //Serial.println(responseMessage);

      } else {
        Serial.print(F("HTTP POST request failed, error: "));
        Serial.println(httpResponseCode);
      }
      http.end();
      checkEmpty = false;
    }
  }
}

#endif
