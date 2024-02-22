# 乐鑫 ESP32-ChatGLM 大模型自定义对话 - [English Docs](https://github.com/blueokanna/ESP32-ChatGLM/blob/main/README.md)
使用乐鑫 ESP32 平台来享受单片机上的开放的大语言模型 **ChatGLM** ！

----

## 关于本次的调用
使用官方的异步调用方式来请求其 API。 ChatGLM API 可从以下网站获取：

API 获取地址：https://open.bigmodel.cn/ 

> 主页如下（可能会更改）：

![Screenshot 2023-10-15 205941](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/b5614ed3-b4e9-43e3-ac01-77c2747f9774)


## 请按照以下步骤操作：

### 步骤 1
下载 **Arduino IDE** 并安装。 打开 `Arduino IDE` 并找到 File -> Perference。
> 请使用 **ESP32** 官方的开发板管理器地址。

### 步骤2
下载项目并下载库。 （内置库的话即可直接使用，否则需要找到相应的资源下载）
这些是该项目的库：
````
#include <Arduino.h>            //内置
#include <CustomJWT.h>          //从库中寻找
#include <ESPAsyncWebServer.h>  //从 https://github.com/me-no-dev/ESPAsyncWebServer 获取
#include <ArduinoJson.h>        //从库中寻找
#include <WiFiClientSecure.h>   //内置
#include <WiFiUdp.h>            //内置
#include <HTTPClient.h>         //从库中寻找
#include <NTPClient.h>          //从库中寻找
#include <queue>                //内置
#include <SPIFFS.h>             //内置
````

### 步骤 3 🤨
找到库文件夹并找到 `CustomJWT` 文件夹。 （我也上传了 `CustomJWT.h` 文件，直接替换成我 **CustomJWT** ☝️）

> **Windows 电脑默认应该是在：C:\Users\xxxx\Documents\Arduino\libraries\CustomJWT\src**

#### CustomJWT.h 文件中的下面的代码不正确：
```
sprintf(headerJSON, "{\"alg\": \"%s\",\"typ\":\"%s\",\"sign_type\":\"%s\"}", alg、typ、sign_type);
```

> 错误： 这是 \"alg\": \"%s\" 之间的问题，在 \"alg\": 和 \"%s\" 之间有一个空格！ **请删除它**！ 如果没有，它将显示不同的 **Base64** 编码。

#### 修复 `CustomJWT.h` 文件中的代码，如下所示：
```
sprintf(headerJSON, "{\"alg\":\"%s\",\"typ\":\"%s\",\"sign_type\":\"%s\"}", alg、typ、sign_type);
```

### 步骤4
将 **ChatGLM.ino** 和 **index.h** 项目放入您自己的项目文件夹中，文件夹名称为：ChatGLM（这里文件夹名称需要和 *.ino 文件保持一致）。你可以更改您的个人 **请求API** 和 **WiFI** ,甚至可以更改 **system_role** 的角色内容以及 **NTP** 网络时间服务器，甚至您可以为**index.h** 文件

### 步骤 5
通过串口和电脑的 USB 连接 **ESP32** 模块。 请选择正确的 **ESP32** 板以及你的 **Arduino** 设置，我的是 **ESP32-S3-N8R2** 的单片机

![图片](https://user-images.githubusercontent.com/56761243/268492784-49fc02d8-060d-4898-9d80-15b4fe50ea07.png)

我的 Arduino 设置为：

![1](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/e1d1d8e2-d888-4c0e-bb44-677de59664eb)


> 终于可以愉快烧录你的 **ESP32** 设备了！ 😄🥇


## 其他问题：
> 目前最新支持 **ChatGLM-4** 的模型，如果需要 **ChatGLM-3-Turbo** 的模型可以下载本项目在 **utiltools.h** 文件进行修改。目前也集成了 AI 角色扮演的内容。默认情况下是使用异步调用且使用的是非角色扮演的 **prompt**，如果您还有其他问题，可以> 发起 **讨论（Discussion）** 或者发起 **Issue**，看到的话，我会为你解答！同时也非常欢迎大家来 **fork** 本项目，如果不介意的话也可以给我的项目点个⭐！ **非常感谢！**
----

最后感谢 Github 的 **@JoinChang** 和其他库开发者👍
