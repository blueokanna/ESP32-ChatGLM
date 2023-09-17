#include <Arduino.h>            //build in
#include <CustomJWT.h>          //find from library
#include <ESPAsyncWebServer.h>  // Get from https://github.com/me-no-dev/ESPAsyncWebServer
#include <ArduinoJson.h>        //find from library
#include <WiFiClientSecure.h>   //build in
#include <WiFiUdp.h>            //build in
#include <time.h>               //build in
#include <HTTPClient.h>         //find from library
#include "index.h"

char header[50];
char payload[500];
char signature[100];
char out[500];
char jsonString[500];

char idCharArray[150];
char secretCharArray[100];

const char *ssid_1 = "xxxxx";
const char *ssid_2 = "xxxxxx";
const char *ssid_3 = "xxxxx";

const char *password_1 = "xxxxxx";
const char *password_2 = "xxxxxxx";
const char *password_3 = "xxxxxxxx";

const char *ssidList[] = { ssid_1, ssid_2, ssid_3 };
const char *passwordList[] = { password_1, password_2, password_3 };
const char *getMessage;

unsigned long timestamp;
time_t now;
struct tm timeinfo;

bool checkEmpty = false;
bool checkIP = true;

String getReturnJson, responseMessage, userMessage;
HTTPClient http, http_id;

char *user_info = "BlueOkanna";
char *bot_info = "Bots info user design" char *bot_name = "Bot_name";
char *user_name = "Blue";
char *role = "user";

char *version = "0.0.1";

char *setApiKey = "xxxxxxxxxxxxxxxxxxxxxxxxxx.xxxxxxxxxxx";  //Get Api key from https://open.bigmodel.cn/

//char* web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/characterglm/async-invoke";  //ChatGLM Character
//char *web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/chatglm_pro/async-invoke";  //ChatGLM Pro
//char* web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/chatglm_std/async-invoke";  //ChatGLM Std
//char *web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/chatglm_lite/async-invoke";  // ChatGLM lite

const char *ntpServer = "ntp.aliyun.com";  //You can change NTP from what you want, the default is aliyun
const long gmtOffset_sec = 28800;          //Change GMT, default is China's NTP +8
const int daylightOffset_sec = 0;

AsyncWebServer server(80);  //Web Page IP:80
DynamicJsonDocument doc(1000);

void initTime() {
  Serial.println("Initializing time synchronization...");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  for (int i = 0; i < 10; i++) {
    if (time(nullptr)) {
      Serial.println(F("Time synchronized"));
      return;
    }
    delay(1000);
    Serial.println(F("Waiting for time synchronization..."));
  }
  Serial.println(F("Time synchronization failed. Check NTP server and network."));
}

void splitApiKey(const char *apikey) {  //IMPORTANT
  const char *delimiter = strchr(apikey, '.');

  if (delimiter != NULL) {
    size_t idLength = delimiter - apikey;

    if (idLength < sizeof(idCharArray)) {
      strncpy(idCharArray, apikey, idLength);
      idCharArray[idLength] = '\0';
      strcpy(secretCharArray, delimiter + 1);
      snprintf(jsonString, sizeof(jsonString), "{\"api_key\":\"%s\",\"exp\":%lu,\"timestamp\":%lu}", idCharArray, now * 5, now);  //expire time is set 5 * now
      CustomJWT jwt(secretCharArray, header, sizeof(header), payload, sizeof(payload), signature, sizeof(signature), out, sizeof(out));
      jwt.encodeJWT(jsonString);
      getReturnJson = jwt.out;  //return jwt
      jwt.clear();
    } else {
      Serial.println("ID part of API key is too long.");
    }
  } else {
    Serial.println("Invalid API key format.");
  }
}

int tryWiFiConnection(const char *ssid, const char *password, int networkNumber) {
  Serial.printf("Connecting to WiFi_%d...\n", networkNumber);

  int attempts = 0;
  const int maxAttempts = 10;

  while (attempts < maxAttempts) {
    WiFi.begin(ssid, password);

    int connectionAttempt = 0;
    while (connectionAttempt < 4) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("Connected to WiFi_%d.\n", networkNumber);
        return networkNumber;
      }
      delay(1000);
      connectionAttempt++;
    }

    Serial.printf("Retry WiFi%d connection...\n", networkNumber);
    WiFi.disconnect();
    attempts++;
  }

  Serial.printf("WiFi%d connection failed.\n", networkNumber);
  return -1;
}

void setup() {
  Serial.begin(115200);
  delay(100);

  for (int networkNumber = 0; networkNumber < 3; networkNumber++) {
    Serial.printf("Connecting to WiFi_%d...\n", networkNumber + 1);

    int successfulConnection = tryWiFiConnection(ssidList[networkNumber], passwordList[networkNumber], networkNumber + 1);

    if (successfulConnection != -1) {
      Serial.printf("Connected to WiFi_%d\n", successfulConnection);
      Serial.print("The Internet IP: ");
      Serial.println(WiFi.localIP());

      initTime();
      if (getLocalTime(&timeinfo)) {
        now = mktime(&timeinfo);
        splitApiKey(setApiKey);
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
          request->send(200, "text/html", html);
        });

        server.on("/receiveTextMessage", HTTP_GET, [](AsyncWebServerRequest *request) {
          const char *getMessage = doc["data"]["task_id"];
          String web_search_id = "https://open.bigmodel.cn/api/paas/v3/model-api/-/async-invoke/" + String(getMessage);  //GET Method(+below)
          http_id.begin(web_search_id);
          http_id.addHeader("Accept", "application/json");
          http_id.addHeader("Content-Type", "application/json; charset=UTF-8");
          http_id.addHeader("Authorization", getReturnJson);
          int httpResponseIDCode = http_id.GET();
          if (httpResponseIDCode > 0) {
            responseMessage = http_id.getString();
          } else {
            responseMessage = "Error: External API request failed!";
          }
          request->send(200, "text/html", responseMessage);
        });

        server.on("/send", HTTP_GET, [](AsyncWebServerRequest *request) {
          responseMessage.clear();
          userMessage = request->getParam("message")->value();
          if (userMessage.length() > 0) {
            checkEmpty = true;
            checkIP = true;
            Serial.print(F("My Question is: "));
            Serial.print(userMessage);
            Serial.println();
          }
          request->send(200, "text/plain", "Message sent to Serial");
        });
        server.begin();
      } else {
        Serial.println(F("Failed to obtain Beijing time"));   //if you change your ntp you had better to change it
      }
      break;
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to any WiFi network.");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    http.begin(web_hook);
    http.addHeader("Accept", "application/json");   
    http.addHeader("Content-Type", "application/json; charset=UTF-8");
    http.addHeader("Authorization", getReturnJson);

    if (checkEmpty) {
      // String metaData = "\"meta\": {\"user_info\": \"" + String(user_info) + "\", \"bot_info\": \"" + String(bot_info) + "\", \"bot_name\": \"" + String(bot_name) + "\", \"user_name\": \"" + String(user_name) + "\"}";
      // String payloadMessage = "{\"prompt\": [{\"role\": \"" + String(role) + "\", \"content\": \"" + userMessage + "\"}], " + metaData + "}";
      String payloadMessage = "{\"prompt\": \"" + userMessage + "\"}";

      int httpResponseCode = http.POST(payloadMessage);
      if (httpResponseCode > 0) {
        String returnJsonMessage = http.getString();
        DeserializationError error = deserializeJson(doc, returnJsonMessage);
        if (error) {
          Serial.print(F("JSON parsing failed: "));
          Serial.println(F(error.c_str()));
        }
      }
    }
    checkEmpty = false;
    http.end();
  }
  delay(100);
}
