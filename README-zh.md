# :school:乐鑫 ESP32 ChatGLM 大模型自定义对话 - [English Docs](https://github.com/blueokanna/ESP32-ChatGLM/blob/main/README.md)
:octocat: 使用乐鑫 ESP32 平台来享受单片机上的开放的大语言模型 **ChatGLM** ！

----

## 关于本次的调用
| Number | Parts | Description |
| :-------------: | :-------------: | :----- |
| 1 | Name | 	:vhs: ESP32 ChatGLM |
| 2 | Version  | 🕸 0.0.5 |
| 3 | Support Audio and Microphone| :loudspeaker: Option (Yes) |
| 4 | Support Web | 🎭 Yes |
| 5 | Support Mobile | 📱 No |

使用官方的异步调用方式来请求其 API。 ChatGLM API 可从以下网站获取：

:paperclip: API 获取地址：https://open.bigmodel.cn/ 

> 	:pencil:主页如下（可能会更改）：

![Screenshot 2023-10-15 205941](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/b5614ed3-b4e9-43e3-ac01-77c2747f9774)


## :one:请按照以下步骤操作⚠️：

### 步骤 1
下载 **Arduino IDE** 并安装。 打开 `Arduino IDE` 并找到 File -> Perference。
> 请使用 **ESP32** 官方的开发板管理器地址。

### 步骤2
下载项目并下载库。 （内置库的话即可直接使用，否则需要找到相应的资源下载）
这些是该项目的库：
````
#include <Arduino.h>            //内置
#include <CustomJWT.h>          //从库中找到安装
#include <ESPAsyncWebServer.h>  //从 https://github.com/me-no-dev/ESPAsyncWebServer 找到下载
#include <ArduinoJson.h>        //从库中找到安装
#include <WiFiClientSecure.h>   //内置
#include <WiFiUdp.h>            //内置
#include <HTTPClient.h>         //内置
#include <NTPClient.h>          //从库中找到安装
#include <queue>                //内置
#include <SPIFFS.h>             //内置
#include <SPI.h>                //内置
#include <SoftwareSerial.h>     //从库中找到安装
#include "UTF8ToGB2312.h"       //从库中找到安装
#include <esp_heap_caps.h>      //内置
#include "base64.h"             //内置
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


> 🤖终于可以愉快烧录你的 **ESP32** 设备了！ 😄🥇

<br>
<br>

# :two: 高级操作：麦克风输入和音频输出:
这里我们将使用 Max9814 麦克风和 SNR9816VR_TTS 模块，您可以在**淘宝（中国）** 和 **阿里巴巴（全球）** 找到这些模块。

**将所有模块连接在一起:**

![IMG_20240418_145303](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/4420cc73-90d2-49f6-baaf-a4797ad01301)

```
#define MYPORT_TX 0      //连接 SNR9816VR_TTS RX_PIN 与 ESP32-S3 PIN_0
#define MYPORT_RX 1      //连接 SNR9816VR_TTS TX_PIN 与 ESP32-S3 PIN_RX
#define audio_out 4      //将 ESP32-S3 PIN_4 与 MAX9814 OUT 连接起来
#define audio_keys 6     //用按钮连接 ESP32-S3 PIN_6（默认为高电平）
#define disk_format 7    //用按钮连接 ESP32-S3 PIN_7（默认为高电平）
```

![max9814-mikrofon-964239](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/ddc6a7d5-23ea-46b5-9c3d-1412338d5049)

> 如果浮空 AR，则语音增益为 60 分贝

> 如果将 AR 连接到 3.3V，它将获得 50 db 的语音增益

> 如果将 AR 连接到地线，则语音增益为 40 分贝    <--- 我的默认选择

## :alarm_clock:onHardWareTimer code here:

该代码是一个处理定时器中断的中断服务程序。audio_out 数据采集逻辑在定时器中断服务例程中实现。当满足条件时（如 audio_start_flag 设置为 1），中断服务例程将从 audio_out 读取数据并存储，直至采集完成。

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
> 如果您想了解这部分内容，可以打开我的项目找到 **<ChatGLM.ino>** 这个文件

### :sound: SNR9816VR_TTS 模块

本模块仅支持 GBK2312 中文版（我的模块不支持英文版，您可以购买英文版的 TTS 模块）。

根据图片显示，您需要 5V 电压来驱动电路板，但我的 ESP32 S3 不支持 5V 电压输出（ESP32C3 有 5V 输出）：
![Screenshot 2024-04-18 160234](https://github.com/blueokanna/ESP32-ChatGLM/assets/56761243/1210638f-a9c5-44f5-9ce0-77a81a2251fa)

### 这里我们只需要语音方法:
该函数将字符串转换为 GB2312 编码格式并发送至串行设备。

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
  for (int i = 1; i < dat_len + 2; i++) {            //遍历报头数组（报头起始标志除外），执行异或运算，并计算校验和值。
    xor_value ^= head[i];                            //将校验和值存储在标头的最后一个字节中
  }
  head[dat_len + 2] = xor_value;

  myPort.write(head, dat_len + 3);

  delay(gb2312_str.length() * 10);                   //延迟一段时间，以确保接收器有足够的时间处理接收到的数据。延迟时间取决于 GB2312 字符串的长度。
}

```


## ❓其他问题：
> 目前最新支持 **ChatGLM-4** 的模型，如果需要 **ChatGLM-3-Turbo** 的模型可以下载本项目在 **utiltools.h** 文件进行修改。目前也集成了 AI 角色扮演的内容。默认情况下是使用异步调用且使用的是非角色扮演的 **prompt**，如果您还有其他问题，可以> 发起 **讨论（Discussion）** 或者发起 **Issue**，看到的话，我会为你解答！同时也非常欢迎大家来 **fork** 本项目，如果不介意的话也可以给我的项目点个⭐！ **非常感谢！**
----

最后感谢 Github 的 **@JoinChang** 和其他库开发者👍

## 💶 捐赠 (时刻感谢你对我的支持! :smile:):
<p><a href="https://www.buymeacoffee.com/blueokanna"> <img align="left" src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" height="50" width="210" alt="https://www.buymeacoffee.com/blueokanna" /></a></p><br><br>
