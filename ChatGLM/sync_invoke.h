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

void loopingSetting(HTTPClient &http, String &LLM, String &JsonToken, String &responseMessage, String &userMessage, String &invokeChoice, bool &checkEmpty) {
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

        String payloadMessage = "{\"model\":\"" + LLM + "\", \"messages\":[{\"role\":\"system\",\"content\":\"" + String(system_role) + "\"},{\"role\":\"user\",\"content\":\"" + userMessage + "\"}],\"stream\":false}";

        int httpResponseCode = http.POST(payloadMessage);
        responseMessage.clear();

        if (httpResponseCode > 0) {
          responseMessage = http.getString();
          //Serial.println(responseMessage);  //debug
          break;
        } else {
          retryCount++;
          Serial.print(F("HTTP POST request failed, error: "));
          Serial.println(httpResponseCode);
          delay(500);
        }
      }
    }
    http.end();
    checkEmpty = false;
  }
}

#endif
