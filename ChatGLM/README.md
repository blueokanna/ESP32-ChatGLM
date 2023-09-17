# ESP32-ChatGLM
Using the Chinese version of ChatGPT -- "ChatGLM" on Espressif's ESP32 platform by Arduino!

# ReadMe
Use the official asynchronous calling method to request its API. The API is obtained from the following websites:

API receive : https://open.bigmodel.cn/

![Image](https://user-images.githubusercontent.com/56761243/268491167-5013b3f0-432b-45cc-8501-dcc2b2a173f8.png)

## Please Following Steps below:

### Steps 1
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
### Step 3(Hard step) 🤨
FInd the library folder and find the CustomJWT Folder.  (I also upload `CustomJWT.h` file☝️)

# `(Remeber Watch here!)`

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

### Step4 
Paste the project for `ChatGLM.ino` and `index.h` into your own project folder and start it to change your personal API and WiFI(You can change your own local the NTP) and even you can design your personal Webpage for `index.h` file

### Step 5
Connect the ESP32 Module by Serial Port and PC's USB. Please choose correct ESP32 board to PC, Mine is ESP32-S3


![Image](https://user-images.githubusercontent.com/56761243/268492784-49fc02d8-060d-4898-9d80-15b4fe50ea07.png)

> Finally Happy Flashing your ESP32 device! 😄🥇 


---

Last thanks the @JoinChang and other libraries providers 👍