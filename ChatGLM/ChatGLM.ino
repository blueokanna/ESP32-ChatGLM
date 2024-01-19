#include <Arduino.h>            //build in
#include <CustomJWT.h>          //find from library
#include <ESPAsyncWebServer.h>  // Get from https://github.com/me-no-dev/ESPAsyncWebServer
#include <ArduinoJson.h>        //find from library
#include <WiFiClientSecure.h>   //build in
#include <WiFiUdp.h>            //build in
#include <HTTPClient.h>         //find from library
#include <NTPClient.h>          //find from library

#include "chatconfig.h"
#include "index.h"
#include "async_invoke.h"
#include "sync_invoke.h"

char header[50];
char payload[500];
char signature[100];
char out[500];
char jsonString[500];

char idCharArray[150];
char secretCharArray[100];


String invokeChoice = "Async_invoke";

//String invokeChoice = "Sync_invoke";

String LLM_Model = "glm-4";

String JsonToken, responseMessage, userMessage;
HTTPClient http, http_id;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 0, 30000);

AsyncWebServer server(9898);  // Web Page IP:9898
DynamicJsonDocument doc(20480);

long long int timestamp_generation() {
  if (wifiConnect) {
    timeClient.update();
    long long int timestamp_generation = timeClient.getEpochTime() * 1000ULL;  //get Timestamp
    return timestamp_generation;
  }
}

void splitApiKey(const char *apikey) {
  const char *delimiter = strchr(apikey, '.');

  if (delimiter != NULL) {
    size_t idLength = delimiter - apikey;

    if (idLength < sizeof(idCharArray)) {
      strncpy(idCharArray, apikey, idLength);
      idCharArray[idLength] = '\0';
      strcpy(secretCharArray, delimiter + 1);
      snprintf(jsonString, sizeof(jsonString), "{\"api_key\":\"%s\",\"exp\":%lld,\"timestamp\":%lld}", idCharArray, static_cast<long long>(timestamp_generation()) * 3, static_cast<long long>(timestamp_generation()));
      CustomJWT jwt(secretCharArray, header, sizeof(header), payload, sizeof(payload), signature, sizeof(signature), out, sizeof(out));
      jwt.encodeJWT(jsonString);
      JsonToken = jwt.out;

      Serial.println(JsonToken);  //Debug

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
      WiFi.begin(ssid, WPA2_AUTH_PEAP, identity, identity, password);  //WPA2_ENTERPRISE | Eduroam calling
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

    if (successfulConnection != -1 && !wifiConnect) {
      Serial.printf("Connected to WiFi_%d\n", successfulConnection);
      Serial.print("The Internet IP: ");
      Serial.println(WiFi.localIP());

      if (timestamp_generation > 0) {
        splitApiKey(setApiKey);

        if (invokeChoice == "Async_invoke") {
          asyncMessage(server, http_id, doc, JsonToken, responseMessage, userMessage, checkEmpty);  //Async
        } else if (invokeChoice == "Sync_invoke") {
          syncMessage(server, responseMessage, userMessage, checkEmpty);  //Sync
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
      loopingSetting(http, LLM_Model, doc, JsonToken, userMessage, invokeChoice, checkEmpty);  //Async
    } else if (invokeChoice == "Sync_invoke") {
      loopingSetting(http, LLM_Model, JsonToken, responseMessage, userMessage, invokeChoice, checkEmpty);  //Sync
    }
  }
  delay(100);
}
