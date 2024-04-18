# :school:ESP32 ChatGLM for Custom Conversations - [中文文档](https://github.com/blueokanna/ESP32-ChatGLM/blob/main/README-zh.md)

> Technological achievements from Tsinghua University --->
> "ChatGLM" 🇨🇳 on Espressif's ESP32 platform by Arduino!

# :octocat: ESP32 ChatGLM Project (Version 0.0.5)
Use the official asynchronous calling method to request its API. The API is obtained from the following websites:

:paperclip: API receive : https://open.bigmodel.cn/

![1](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/2224c46a-0d0d-4c5e-8a09-cdfaf2c7cc46)

## :one:Please Following Steps below⚠️:

### Step 1
Download Arduino IDE and install it. Open the IDE and find the File -> Perference. 
> Please Use ESP32 Addritional boards manager URLs. 

### Step 2
Download the Project and Download libraries. (Build in just use it, otherwise download it)
These are libraries for this project:
```
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
```

### Step 3 🤨
FInd the library folder and find the CustomJWT Folder.  (I also upload `CustomJWT.h` file, Directly replace with me **CustomJWT** ☝️)

Find the CustomJWT.h file in src folder. to add some code in CustomJWT(........) (in brackets)

`char *sign_type = "SIGN"` add under `char *typ = "JWT" ` for each CustomJWT method!

![Image](https://user-images.githubusercontent.com/56761243/268493130-475f39f9-f6c4-4721-b6d5-c566be82e2c8.png)

`IMPORTANT!` 
there got a problem for Library JWT library(The Library name called: CustomJWT)

#### Incorrect Code in CustomJWT.h file
```sprintf(headerJSON, "{\"alg\": \"%s\",\"typ\":\"%s\",\"sign_type\":\"%s\"}", alg, typ, sign_type);``` 

`Error:` The is a problem between \"alg\": \"%s\"which has a space between \"alg\": and \"%s\"! Please delete it! if not it will show different Base64 code. 

#### Fix the Code in `CustomJWT.h` file
```sprintf(headerJSON, "{\"alg\":\"%s\",\"typ\":\"%s\",\"sign_type\":\"%s\"}", alg, typ, sign_type);```

### Step4 📄
Place the **ChatGLM.ino** and **index.h** projects into your own project folder with the name: ChatGLM (here the folder name needs to be the same as the *.ino file). You can change your personal **Request API** and **WiFI**, even change the role content of **system_role** and **NTP** network time server, and you can even add a new **index.h** file to **index.h**.

### Step 5 💻
Connect the ESP32 Module by Serial Port and PC's USB. Please choose correct ESP32 board to PC, Mine is **YD-ESP32-S3-N16R8(v1.3)**


![Image](https://user-images.githubusercontent.com/56761243/268492784-49fc02d8-060d-4898-9d80-15b4fe50ea07.png)

And my Arduino Setting is：

![Screenshot 2024-04-18 153008](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/e4cbc1a5-de06-4cfd-8fe7-a57c41ca00de)


> Finally Happy Flashing your ESP32 device! 😄🥇

<br>
<br>

# :two:Advanced with Microphone and Audio:
The programme use Max9814 Microphone and SNR9816VR_TTS Module which you can find in **Taobao (China)** and **Alibaba(Global)**

**Connect all Module together:**
![IMG_20240418_145303](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/4420cc73-90d2-49f6-baaf-a4797ad01301)

```
#define MYPORT_TX 0      //Connect SNR9816VR_TTS RX_PIN and Connect ESP32-S3 PIN_0
#define MYPORT_RX 1      //Connect SNR9816VR_TTS TX_PIN and Connect ESP32-S3 PIN_RX
#define audio_out 4      //Connect ESP32-S3 PIN_4 with MAX9814 OUT
#define audio_keys 6     //Connect ESP32-S3 PIN_6 with button(Default high level) 
#define disk_format 7    //Connect ESP32-S3 PIN_7 with button(Default high level) 
```

![max9814-mikrofon-964239](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/ddc6a7d5-23ea-46b5-9c3d-1412338d5049)

> If you flow AR that, it's gain 60db of Voice
> 
> If you connect AR to Ground, it's gain 50db of Voice    <--- I did

> If you connect AR to 3.3V, it's gain 40 db of Voice


## :alarm_clock:onHardWareTimer code here:

This code is an interrupt servicer that handles timer interrupts. The logic for ADC data acquisition is implemented in the timer interrupt service routine. When a condition is met (e.g. audio_start_flag is set to 1), the interrupt service routine reads data from audio_out and stores it until the acquisition is complete.

```
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
```
> If you want to know this part, you can open my project to find it **<ChatGLM.ino>**.

### :sound: SNR9816VR_TTS Module

This Module is only support GBK2312 for Chinese (Mine is not support English, you may bought English version of this board)

Base on picture shows that you need 5V to drive the board but ESP32 S3 for me doesn't support 5V (ESP32C3 got 5V output):
![Screenshot 2024-04-18 160234](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/1210638f-a9c5-44f5-9ce0-77a81a2251fa)

### Here we just need speech method:
This function converts the string into GB2312 encoded format and sends it to the serial device.

```
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
  for (int i = 1; i < dat_len + 2; i++) {            // Iterates through the header array (except for the header start flag), performs a different-or operation, and computes the checksum value.
    xor_value ^= head[i];                            // Store the checksum value in the last byte of the header
  }
  head[dat_len + 2] = xor_value;

  myPort.write(head, dat_len + 3);

  delay(gb2312_str.length() * 10);                   // Delayed for a period of time to ensure that the receiver has enough time to process the received data. The delay here is determined by the length of the GB2312 string
}

```

## Other Questions:
> Currently supports **ChatGLM-4** models, if you need **ChatGLM-3-Turbo** models you can download this project and modify it in **utiltools.h** file. AI roleplaying is also currently integrated. By default, it uses asynchronous calls and non-role-playing **prompt**, if you have other questions, you can handin a **Discussion** or start an **Issue** and I will answer your questions if I see them as soon as possible! You are also very welcome to **fork** this project, and if you don't mind you can also give my project a ⭐! **Thanks!**
---


Last thanks to @JoinChang and other libraries providers 👍

## 💶 Donation (Thank you all the time! :smile:):
<p><a href="https://www.buymeacoffee.com/blueokanna"> <img align="left" src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" height="50" width="210" alt="https://www.buymeacoffee.com/blueokanna" /></a></p><br><br>

