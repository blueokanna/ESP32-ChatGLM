# ESP32-ChatGLM for Custom Conversations - [中文文档](https://github.com/blueokanna/ESP32-ChatGLM/blob/main/README-zh.md)

> Technological achievements from Tsinghua University --->
> "ChatGLM" 🇨🇳 on Espressif's ESP32 platform by Arduino!

# ESP32-ChatGLM Project
Use the official asynchronous calling method to request its API. The API is obtained from the following websites:

API receive : https://open.bigmodel.cn/

![1](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/2224c46a-0d0d-4c5e-8a09-cdfaf2c7cc46)

## Please Following Steps below⚠️:

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
#include <time.h>               //build in
#include <HTTPClient.h>         //find from library
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
Connect the ESP32 Module by Serial Port and PC's USB. Please choose correct ESP32 board to PC, Mine is ESP32-S3


![Image](https://user-images.githubusercontent.com/56761243/268492784-49fc02d8-060d-4898-9d80-15b4fe50ea07.png)

> Finally Happy Flashing your ESP32 device! 😄🥇 

## Other Questions:
> Currently the latest support for ChatGLM-4 with integrated AI roleplaying content, this one removes the SSE requests from version 0.0.2. Since the official documentation is a large-scale revision that basically combines streaming and synchronous requests into one, I'll leave the synchronous requests here, and if you have any other questions, you can start a **Discussion** or start an **Issue**, and I'll answer them for you when I see them! Also fork this project thanks!
---

Last thanks to @JoinChang and other libraries providers 👍
