#include <Arduino.h>            //build in
#include <CustomJWT.h>          //find from library
#include <ESPAsyncWebServer.h>  // Get from https://github.com/me-no-dev/ESPAsyncWebServer
#include <ArduinoJson.h>        //find from library
#include <WiFiClientSecure.h>   //build in
#include <WiFiUdp.h>            //build in
#include <time.h>               //build in
#include <HTTPClient.h>         //find from library

#include "chatconfig.h"
#include "index.h"
#include "async_invoke.h"
#include "sync_invoke.h"
#include "sse_invoke.h"

char header[50];
char payload[500];
char signature[100];
char out[500];
char jsonString[500];

char idCharArray[150];
char secretCharArray[100];

unsigned long timestamp;
time_t now;
struct tm timeinfo;

String invokeChoice = "Async_invoke"; //Set the Voking Method for project(Support Sync and SSE)
//String invokeChoice = "Sync_invoke";
//String invokeChoice = "SSE_invoke";

String JsonToken, responseMessage, userMessage;
HTTPClient http, http_id;

AsyncWebServer server(9898);  // Web Page IP:9898
DynamicJsonDocument doc(20480);
DynamicJsonDocument jsonDoc(20480);

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

void splitApiKey(const char *apikey) {
  const char *delimiter = strchr(apikey, '.');

  if (delimiter != NULL) {
    size_t idLength = delimiter - apikey;

    if (idLength < sizeof(idCharArray)) {
      strncpy(idCharArray, apikey, idLength);
      idCharArray[idLength] = '\0';
      strcpy(secretCharArray, delimiter + 1);
      snprintf(jsonString, sizeof(jsonString), "{\"api_key\":\"%s\",\"exp\":%lu,\"timestamp\":%lu}", idCharArray, now * 3, now);  // expire time is set 3 * now
      CustomJWT jwt(secretCharArray, header, sizeof(header), payload, sizeof(payload), signature, sizeof(signature), out, sizeof(out));
      jwt.encodeJWT(jsonString);
      JsonToken = jwt.out;
      jwt.clear();
    } else {
      Serial.println("ID part of API key is not valid.");
    }
  } else {
    Serial.println("Invalid API key format.");
  }
}

int tryWiFiConnection(const char *ssid, const char *identity, const char *password, int networkNumber) {
  Serial.printf("Connecting to WiFi_%d...\n", networkNumber);

  int attempts = 0;
  const int maxAttempts = 10;

  while (attempts < maxAttempts) {
    if (strcmp(identity, "none") == 0) {
      WiFi.begin(ssid, password);
    } else {
      WiFi.begin(ssid, WPA2_AUTH_PEAP, identity, identity, password); //WPA2_ENTERPRISE | Eduroam calling
    }

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

  for (int networkNumber = 0; networkNumber < 3 && !wifiConnect; networkNumber++) {
    Serial.printf("Connecting to WiFi_%d...\n", networkNumber + 1);

    int successfulConnection = tryWiFiConnection(ssidList[networkNumber], identityList[networkNumber], passwordList[networkNumber], networkNumber + 1);

    if (successfulConnection != -1) {
      Serial.printf("Connected to WiFi_%d\n", successfulConnection);
      Serial.print("The Internet IP: ");
      Serial.println(WiFi.localIP());

      initTime();
      if (getLocalTime(&timeinfo)) {
        now = mktime(&timeinfo);
        splitApiKey(setApiKey);


        if (invokeChoice == "Async_invoke") {
          asyncMessage(server, http_id, doc, JsonToken, responseMessage, userMessage, checkEmpty); //Async
          invokeChoice = "Async_invoke";
        } else if (invokeChoice == "Sync_invoke") {
          syncMessage(server, JsonToken, responseMessage, userMessage, checkEmpty); //Sync
          invokeChoice = "Sync_invoke";
        } else if (invokeChoice == "SSE_invoke") {
          SSEMessage(server, JsonToken, responseMessage, userMessage, checkEmpty); //SSE
          invokeChoice = "SSE_invoke";
        }

        wifiConnect = true;
      } else {
        Serial.println(F("Failed to obtain Beijing time"));
      }
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to any WiFi network.");
  }
}

void loop() {
  if (wifiConnect && WiFi.status() == WL_CONNECTED) {
    if (invokeChoice == "Async_invoke") {
      loopingSetting(http, doc, JsonToken, userMessage, invokeChoice, checkEmpty);  //Async
    } else if (invokeChoice == "Sync_invoke") {
      loopingSetting(http, JsonToken, responseMessage, userMessage, invokeChoice, checkEmpty);  //Sync
    } else if (invokeChoice == "SSE_invoke") {
      loopingSetting(http, JsonToken, jsonDoc, responseMessage, userMessage, invokeChoice, checkEmpty);  //SSE
    }
  }
  delay(100);
}
