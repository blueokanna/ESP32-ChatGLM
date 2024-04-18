#include <Arduino.h>            //build in
#include <CustomJWT.h>          //find from library
#include <ESPAsyncWebServer.h>  // Get from https://github.com/me-no-dev/ESPAsyncWebServer
#include <ArduinoJson.h>        //find from library
#include <WiFiClientSecure.h>   //build in
#include <WiFiUdp.h>            //build in
#include <HTTPClient.h>         //build in
#include <NTPClient.h>          //find from library
#include <queue>                //build in
#include <SPIFFS.h>             //build in
#include <SPI.h>                //build in
#include <SoftwareSerial.h>     //find from library
#include "UTF8ToGB2312.h"       //find from library
#include <esp_heap_caps.h>      //build in
#include "base64.h"             //build in

#define MYPORT_TX 0
#define MYPORT_RX 1
#define audio_out 4
#define audio_keys 6
#define disk_format 7
#define data_len 16000

uint8_t audio_start_flag = 0;
uint8_t audio_complete_flag = 0;

volatile uint16_t audio_output[data_len];
char audio_data_json[45000];

EspSoftwareSerial::UART myPort;

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

hw_timer_t *timer = NULL;
uint32_t var_num = 0;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

String invokeChoice = "Sync_invoke";  //Invoke Mode Default choice

String responseTempMessages;
String JsonToken, responseMessage, userMessage, baidu_voice_token;
HTTPClient http, http_id, http_audio_client, baidu_client;

std::queue<String> messagesQueue;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 0, 30000);  //TimeStamp fetch from NTP

AsyncWebServer server(9898);  // Web Page IP:9898

DynamicJsonDocument doc(51200);
DynamicJsonDocument accessToken(51200);
DynamicJsonDocument audio_decode(102400);
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

  Serial.println(F("Failed to mount LittleFS after multiple attempts! Please reboot or reflash your device!"));
  return false;
}


void baidu_access_token() {
  int httpCode;
  String url = String("https://aip.baidubce.com/oauth/2.0/token?grant_type=client_credentials&client_id=") + String(baidu_api) + String("&client_secret=") + String(baidu_secret);
  baidu_client.begin(url);
  httpCode = baidu_client.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String baidu_payload = baidu_client.getString();
      deserializeJson(accessToken, baidu_payload);
      baidu_voice_token = accessToken["access_token"].as<String>();
      Serial.println(baidu_voice_token);
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", baidu_client.errorToString(httpCode).c_str());
  }
  baidu_client.end();
  delay(100);
}

void IRAM_ATTR onHardWareTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  if (audio_start_flag == 1) {
    audio_output[var_num] = analogRead(audio_out);
    var_num++;
    if (var_num >= data_len) {
      audio_complete_flag = 1;
      audio_start_flag = 0;
      var_num = 0;
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void speech(String data) {
  String gb2312_str = GB.get(data);

  unsigned int dat_len = gb2312_str.length() + 3;

  unsigned char head[dat_len + 3];
  head[0] = 0xFD;          // head byte
  head[1] = dat_len >> 8;  // length byte 1
  head[2] = dat_len;       // length byte 2
  head[3] = 0x01;          // cmd byte
  head[4] = 0x01;          // para byte

  for (int i = 0; i < gb2312_str.length(); i++) {
    head[i + 5] = gb2312_str[i];
  }

  unsigned char xor_value = head[0];
  for (int i = 1; i < dat_len + 2; i++) {
    xor_value ^= head[i];
  }
  head[dat_len + 2] = xor_value;

  myPort.write(head, dat_len + 3);

  delay(gb2312_str.length() * 10);
}

void playAudioTask(void *parameter) {
  const char *audio = (const char *)parameter;
  speech(audio);
  vTaskDelete(NULL);
}

void audio_sync_Setting(String deaudio) {
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

        int httpResponseCode = http.POST(messageJSON(deaudio, false));
        responseMessage.clear();

        if (httpResponseCode > 0) {
          String responseAnswer = http.getString();

          DeserializationError error = deserializeJson(doc, responseAnswer);
          if (!error) {
            responseTempMessages = doc["choices"][0]["message"]["content"].as<String>();

            responseTempMessages.replace("\"", "");
            responseTempMessages.replace("\\n\\n", "");
            responseTempMessages.replace("\\nn", "");
            responseTempMessages.replace("\\n", "");
            responseTempMessages.replace("\\", "");
            responseTempMessages.replace("\null", "");
            responseTempMessages.replace("null", "");
            responseTempMessages.replace("\“", "\"");
            responseTempMessages.replace("\”", "\"");
            responseTempMessages.replace("\？", "\?");
            responseTempMessages.replace("\!", "\!");
            responseTempMessages.replace("\《", "");
            responseTempMessages.replace("\》", "");

            responseTempMessages.trim();
            Serial.println(responseTempMessages);

            speech(responseTempMessages);
            addHistoryToFile(createDoc, assistant_role, responseTempMessages);
            //xTaskCreate(playAudioTask, "Audio Task", responseTempMessages.length() + 10240, (void *)responseTempMessages.c_str(), 1, NULL);
          }
          if (FileSizeChecker()) {
            return;
          }
          //Serial.println(responseMessage);  //debug
          break;
        } else {
          retryCount++;
          Serial.print(F("HTTP POST request failed, error: "));
          Serial.println(httpResponseCode);
          delay(500);  // Retry interval can be adjusted on demand
        }
      }
    }
    http.end();
    checkEmpty = false;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(audio_out, ANALOG);
  pinMode(audio_keys, INPUT_PULLUP);
  pinMode(disk_format, INPUT_PULLUP);

  myPort.begin(115200, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);
  if (!myPort) {  // If the object did not initialize, then its configuration is invalid
    Serial.println("Invalid EspSoftwareSerial pin configuration, check config");
    while (1) {  // Don't continue with invalid configuration
      delay(100);
    }
  }
  delay(100);

  for (int variables_voice_nums = 0; variables_voice_nums < sizeof(tts_voice) / sizeof(tts_voice[0]); variables_voice_nums++) {
    myPort.write(tts_voice[variables_voice_nums]);
  }
  delay(100);

  for (int variables_speed_nums = 0; variables_speed_nums < sizeof(tts_speed) / sizeof(tts_speed[0]); variables_speed_nums++) {
    myPort.write(tts_speed[variables_speed_nums]);
  }
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

        baidu_access_token();

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

  timer = timerBegin(0, 80, true);    //  80M的时钟 80分频 1M
  timerAlarmWrite(timer, 125, true);  //  1M  计125个数进中断  8K
  timerAttachInterrupt(timer, &onHardWareTimer, true);
  timerAlarmEnable(timer);
  timerStop(timer);
}


void loop() {
  if (digitalRead(disk_format) == LOW && beginSPIFFS()) {
    SPIFFS.remove(HistoryfilePath);
    delay(100);
    createFileIfNotExists(HistoryfilePath);
  }
  if (wifiConnect && WiFi.status() == WL_CONNECTED) {
    if (digitalRead(audio_keys) == LOW) {
      Serial.println(F("Start recognition"));

      audio_start_flag = 1;
      timerStart(timer);

      while (!audio_complete_flag) {
        yield();
      }

      timerStop(timer);
      audio_complete_flag = 0;

      memset(audio_data_json, '\0', strlen(audio_data_json));
      snprintf_P(audio_data_json, sizeof(audio_data_json),
                 PSTR("{\"format\":\"pcm\",\"rate\":8000,\"dev_pid\":1537,\"channel\":1,\"cuid\":\"1145141919810abcd\",\"token\":\"%s\",\"len\":32000,\"speech\":\"%s\"}"),
                 baidu_voice_token.c_str(), base64::encode((uint8_t *)audio_output, sizeof(audio_output)).c_str());

      http_audio_client.begin("https://vop.baidu.com/server_api");
      http_audio_client.addHeader("Content-Type", "application/json");
      int httpCode = http_audio_client.POST(audio_data_json);

      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          String audio_payload = http_audio_client.getString();
          deserializeJson(audio_decode, audio_payload);

          String deaudio = audio_decode["result"][0];
          Serial.println(deaudio);  // Debug and Test

          checkEmpty = true;
          audio_sync_Setting(deaudio);
          audio_decode.clear();
          memset((void *)audio_output, '\0', sizeof(audio_output));
        }
      } else {
        Serial.printf_P(PSTR("[HTTP] POST... failed, error: %s\n"), http_audio_client.errorToString(httpCode).c_str());
      }
      http_audio_client.end();
      while (!digitalRead(audio_keys))
        ;
      Serial.printf("Complete recognition!\r\n");
      audio_start_flag = 0;
    }

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
