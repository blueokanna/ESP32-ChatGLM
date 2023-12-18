#ifndef chatconfig_H
#define chatconfig_H

const char *ssid_1 = "eduroam-xxxxxxx";
const char *ssid_2 = "xxxxxxxxxxxxxxxxx";
const char *ssid_3 = "xxxxxxxxxxxxxxxxx";

const char *identity_1 = "YOUR_IDENTITY";
const char *identity_2 = "none";
const char *identity_3 = "none";

const char *password_1 = "xxxxxxxxxxxxxx";
const char *password_2 = "xxxxxxxxxxxxxx!";
const char *password_3 = "xxxxxxxxxxxxxx";


const char *ssidList[] = { ssid_1, ssid_2, ssid_3 };
const char *identityList[] = { identity_1, identity_2, identity_3 };
const char *passwordList[] = { password_1, password_2, password_3 };

const char *user_info = "BlueOkanna";
const char *bot_info = "Blueokanna's Chat Bots";
const char *bot_name = "Bot_name";
const char *user_name = "Blueokanna";
const char *role = "user";

const char *version = "0.0.2";
const char *setApiKey = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.xxxxxxxxxxxxxx";  //Get Api key from https://open.bigmodel.cn/

//const char *sse_web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/chatglm_turbo/sse-invoke";      //New ChatGLM3 SSE
//const char *async_web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/chatglm_turbo/async-invoke";  //New ChatGLM3 async
//const char *sync_web_hook = "https://open.bigmodel.cn/api/paas/v3/model-api/chatglm_turbo/invoke";         //New ChatGLM3 sync

const char *ntpServer = "ntp.aliyun.com";  //You can change NTP from what you want, the default is aliyun
const long gmtOffset_sec = 28800;          //Change GMT, default is China's NTP +8
const int daylightOffset_sec = 0;

bool checkEmpty = false;
bool wifiConnect = false;


#endif