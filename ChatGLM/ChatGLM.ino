#include <Arduino.h>            //build in
#include <CustomJWT.h>          //find from library
#include <ESPAsyncWebServer.h>  // Get from https://github.com/me-no-dev/ESPAsyncWebServer
#include <ArduinoJson.h>        //find from library
#include <WiFiClientSecure.h>   //build in
#include <WiFiUdp.h>            //build in
#include <HTTPClient.h>         //find from library
#include <NTPClient.h>          //find from library
#include <queue>                //build in
#include <SPIFFS.h>             //build in

#include "chatconfig.h"
#include "utiltools.h"
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


String invokeChoice = "Async_invoke";               //Invoke Mode Default choice

//String invokeChoice = "SSE_invoke";

String JsonToken, responseMessage, userMessage;
HTTPClient http, http_id;

std::queue<String> messagesQueue;


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 0, 30000);             //TimeStamp fetch from NTP

AsyncWebServer server(9898);  // Web Page IP:9898

DynamicJsonDocument doc(51200);
DynamicJsonDocument HistoryDoc(204800);
DynamicJsonDocument formattedDoc(204800);
DynamicJsonDocument createDoc(327680);


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
      snprintf(jsonString, sizeof(jsonString), "{\"api_key\":\"%s\",\"exp\":%lld,\"timestamp\":%lld}", idCharArray, static_cast<long long>(timestamp_generation()) * 2, static_cast<long long>(timestamp_generation()));
      CustomJWT jwt(secretCharArray, header, sizeof(header), payload, sizeof(payload), signature, sizeof(signature), out, sizeof(out));
      jwt.encodeJWT(jsonString);
      JsonToken = jwt.out;

      //Serial.println(JsonToken);  //Debug

      jwt.clear();
    } else {
      Serial.println(F("ID part of API key is not valid."));
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

void createFileIfNotExists(const char *path) {
  // Check File is exist
  if (!SPIFFS.exists(path)) {
    Serial.println("File doesn't exist. Creating...");

    File file = SPIFFS.open(path, "w");
    if (!file) {
      Serial.println(F("Failed to create file"));
      return;
    }
    Serial.println(F("File created successfully"));
    file.close();
  } else {
    Serial.println(F("File already exists"));
  }
}

bool beginSPIFFS() {
  int attempt = 0;
  const int maxAttempts = 3;

  while (attempt < maxAttempts) {
    if (SPIFFS.begin()) {
      Serial.println(F("SPIFFS Mount Successful"));
      return true;
    } else {
      Serial.println(F("SPIFFS Mount Failed"));
      Serial.println(F("Attempting to format SPIFFS..."));

      if (SPIFFS.format()) {
        Serial.println(F("SPIFFS formatted successfully"));
      } else {
        Serial.println(F("Error formatting SPIFFS"));
        return false;
      }
    }

    attempt++;
  }

  Serial.println(F("Failed to mount SPIFFS after multiple attempts! Please reboot or reflash your device!"));
  return false;
}



void setup() {
  Serial.begin(115200);
  delay(100);

  for (int networkNumber = 0; networkNumber < 3 && !wifiConnect; networkNumber++) {
    Serial.printf("Connecting to WiFi_%d...\n", networkNumber + 1);

    int successfulConnection = tryWiFiConnection(ssidList[networkNumber], identityList[networkNumber], passwordList[networkNumber], networkNumber + 1);

    if (successfulConnection != -1 && !wifiConnect) {
      Serial.printf("Connected to WiFi_%d\n", successfulConnection);
      Serial.print(F("The Internet IP: "));
      Serial.println(WiFi.localIP());

      if (timestamp_generation > 0) {
        splitApiKey(setApiKey);

        if (!beginSPIFFS()) {
          return;
        }

        if (invokeChoice == "Async_invoke") {
          asyncMessage(server, http_id, doc, formattedDoc, createDoc, JsonToken, responseMessage, userMessage, checkEmpty);  //Async
        } else if (invokeChoice == "Sync_invoke") {
          syncMessage(server, responseMessage, userMessage, checkEmpty);  //Sync
        } else if (invokeChoice == "SSE_invoke") {
          sseMessage(server, responseMessage, userMessage, checkEmpty);  //SSE
        }

        createFileIfNotExists(HistoryfilePath);


        wifiConnect = true;
      } else {
        Serial.println(F("Failed to obtain Beijing time"));
      }
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("Failed to connect to any WiFi network."));
  }
}



void loop() {
  if (wifiConnect && WiFi.status() == WL_CONNECTED) {
    if (userMessage.length() > 0) {
      if (invokeChoice == "Async_invoke") {
        looping_async_Setting(http, doc, JsonToken, userMessage, invokeChoice, checkEmpty);  //Async
      } else if (invokeChoice == "Sync_invoke") {
        looping_sync_Setting(http, doc, formattedDoc, createDoc, JsonToken, responseMessage, userMessage, invokeChoice, checkEmpty);  //Sync
      } else if (invokeChoice == "SSE_invoke") {
        looping_SSE_Setting(http, doc, formattedDoc, createDoc, messagesQueue, JsonToken, responseMessage, userMessage, invokeChoice, checkEmpty);  //SSE
      }
    }
  }
  delay(100);
}
