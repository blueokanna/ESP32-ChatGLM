#ifndef ASYNC_INVOKE_H
#define ASYNC_INVOKE_H

void asyncMessage(AsyncWebServer &server, HTTPClient &http_id, DynamicJsonDocument &doc, String &JsonToken, String &responseMessage, String &userMessage, bool &checkEmpty) {

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


  server.on("/receiveTextMessage", HTTP_GET, [&http_id, &doc, &JsonToken, &responseMessage](AsyncWebServerRequest *request) {
    const char *getMessage = doc["data"]["task_id"];
    String web_search_id = "https://open.bigmodel.cn/api/paas/v3/model-api/-/async-invoke/" + String(getMessage);  //GET Method(+below)

    http_id.begin(web_search_id);
    http_id.addHeader("Accept", "application/json");
    http_id.addHeader("Content-Type", "application/json; charset=UTF-8");
    http_id.addHeader("Authorization", JsonToken);
    int httpResponseIDCode = http_id.GET();
    if (httpResponseIDCode > 0) {
      responseMessage = http_id.getString();
    } else {
      responseMessage = "Error: External API request failed!";
    }
    request->send(200, "text/html", responseMessage);
  });

  server.begin();
}

void loopingSetting(HTTPClient &http, DynamicJsonDocument &doc, String &JsonToken, String &userMessage, String &invokeChoice, bool &checkEmpty) {
  const char *async_web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/chatglm_turbo/async-invoke";  //New ChatGLM3 async

  if (invokeChoice == "Async_invoke") {
    if (checkEmpty) {
      http.begin(async_web_hook);
      http.addHeader("Accept", "application/json");
      http.addHeader("Content-Type", "application/json; charset=UTF-8");
      http.addHeader("Authorization", JsonToken);
      // String metaData = "\"meta\": {\"user_info\": \"" + String(user_info) + "\", \"bot_info\": \"" + String(bot_info) + "\", \"bot_name\": \"" + String(bot_name) + "\", \"user_name\": \"" + String(user_name) + "\"}";
      // String payloadMessage = "{\"prompt\": [{\"role\": \"" + String(role) + "\", \"content\": \"" + userMessage + "\"}], " + metaData + "}";
      String payloadMessage = "{\"prompt\": \"" + userMessage + "\"}";
      // String payloadMessage = "{\"prompt\": [{\"role\": \"" + String(role) + "\", \"content\": \"" + userMessage + "\"}], \"temperature\": \"" + temperature + "\", \"top_p\": \"" + top_p + "\"}";

      int httpResponseCode = http.POST(payloadMessage);
      if (httpResponseCode > 0) {
        String messages = http.getString();
        DeserializationError error = deserializeJson(doc, messages);
        if (error) {
          Serial.print(F("JSON parsing failed: "));
          Serial.println(F(error.c_str()));
        }
      }
    }
    checkEmpty = false;
    http.end();
  }
}

#endif