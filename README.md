# CoolQ HTTP API 插件

[![License](https://img.shields.io/badge/license-GPLv3-blue.svg)](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/LICENSE)
[![Build Status](https://travis-ci.org/richardchien/coolq-http-api.svg)](https://travis-ci.org/richardchien/coolq-http-api)
[![Release](https://img.shields.io/github/release/richardchien/coolq-http-api.svg)](https://github.com/richardchien/coolq-http-api/releases)
[![Download Count](https://img.shields.io/github/downloads/richardchien/coolq-http-api/total.svg)](https://github.com/richardchien/coolq-http-api/releases)
[![Docker Repository](https://img.shields.io/badge/docker-richardchien/cqhttp-blue.svg)](https://hub.docker.com/r/richardchien/cqhttp/)
[![Docker Pulls](https://img.shields.io/docker/pulls/richardchien/cqhttp.svg)](https://hub.docker.com/r/richardchien/cqhttp/)
[![QQ群](https://img.shields.io/badge/qq%E7%BE%A4-201865589-orange.svg)](https://jq.qq.com/?_wv=1027&k=5Euplde)
[![Telegram](https://img.shields.io/badge/telegram-chat-blue.svg)](https://t.me/cqhttp)

通过 HTTP 对酷 Q 的事件进行上报以及接收 HTTP 请求来调用酷 Q 的 DLL 接口，从而可以使用其它语言编写酷 Q 插件。现已支持 WebSocket。

## 使用方法

使用方法见 [https://cqhttp.cc/docs/](https://cqhttp.cc/docs/)。v1.x、v2.x 的旧版本文档在 [https://cqhttp.cc/docs/legacy/](https://cqhttp.cc/docs/legacy/)。

如果访问这个文档非常慢，请尝试国内的地址 [http://richardchien.gitee.io/coolq-http-api/docs/](http://richardchien.gitee.io/coolq-http-api/docs/)。

## SDK

对于下面这些语言的开发者，如果不想自己处理繁杂的请求和解析操作，可以尝试社区中开发者们已经封装好的的 SDK：

| 语言 | Web 框架 | 通信方式 | 地址 | 作者 |
| --- | ------- | ------- | --- | ---- |
| PHP | - | HTTP | https://github.com/kilingzhang/coolq-php-sdk | kilingzhang |
| Python | Bottle | HTTP | https://github.com/richardchien/cqhttp-python-sdk | richardchien |
| Node.js | Koa | HTTP | https://github.com/richardchien/cqhttp-node-sdk | richardchien |
| Node.js | WebSocket-Node | WebSocket | https://github.com/momocow/node-cq-websocket | momocow |
| Java | - | HTTP<br>WebSocket | https://github.com/yangjinhe/maintain-robot | yangjinhe |
| Go | - | HTTP | https://github.com/juzi5201314/cqhttp-go-sdk | juzi5201314 |
| PHP | Swoole | WebSocket | https://github.com/BlueWhaleNetwork/CQBot-swoole | BlueWhaleNetwork |
| Go | Gorilla WebSocket | WebSocket | https://github.com/ma6254/go-cqhttp | ma6254 |
| Java | JFinal | HTTP | https://github.com/thevsk/cqhttp-java-jfinal-sdk | thevsk |

## 应用案例

QQ 机器人可以用来做很多有意思的事情，下面列出一些基于本插件的应用案例：

| 项目地址 | 简介 |
| ------- | --- |
| [CCZU-DEV/xiaokai-bot](https://github.com/CCZU-DEV/xiaokai-bot) | 用 Python 编写的即时聊天平台机器人，通过适配器模式支持使用多种 bot 框架／平台作为消息源（目前支持 Mojo-Webqq、Mojo-Weixin、CoolQ HTTP API），支持自定义插件 |
| [jqqqqqqqqqq/coolq-telegram-bot](https://github.com/jqqqqqqqqqq/coolq-telegram-bot) | QQ <-> Telegram Bot Framework & Forwarder |
| [Mother-Ship/cabbageWeb](https://github.com/Mother-Ship/cabbageWeb) | 基于 Java Web 的 osu! 游戏数据查询机器人 |
| [Ray-Eldath/Avalon](https://github.com/Ray-Eldath/Avalon) | 多功能、可扩展的 QQ 群机器人 |
| [bdbai/Kobirt](https://github.com/bdbai/Kobirt) | Ingress 游戏辅助机器人 |
| [JRT-FOREVER/hub2coolq](https://github.com/JRT-FOREVER/hub2coolq) | GitHub webhook 消息转发至 QQ 群 |

## 修改、编译

项目使用 CMake 构建，[`scripts/generate.ps1`](scripts/generate.ps1)、[`scripts/build.ps1`](scripts/build.ps1)、[`scripts/post_build.ps1`](scripts/post_build.ps1) 分别给出了生成、构建、安装的脚本，你可能需要对它们中的一些变量做适当修改以在你的系统中运行。

除了 [`io.github.richardchien.coolqhttpapi.json`](io.github.richardchien.coolqhttpapi.json) 文件为 GBK 编码，其它代码文件均为 UTF-8 编码。

项目的依赖项通过 [vcpkg](https://github.com/Microsoft/vcpkg) 管理，使用 triplet 如下：

```cmake
set(VCPKG_TARGET_ARCHITECTURE x86)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_PLATFORM_TOOLSET v141)
```

创建了这个 triplet 之后，你需要将 [`scripts/generate.ps1`](scripts/generate.ps1) 中的 `$vcpkg_root` 和 `$vcpkg_triplet` 设置成你系统中的相应值。

除此之外，还需要安装如下依赖（使用上面的 triplet）：

| 模块 | 依赖项 |
| --- | ----- |
| `cqsdk` | `boost-algorithm`<br>`boost-filesystem`<br>`libiconv` |
| `cqhttp` | `cqsdk` 的依赖项<br>`nlohmann-json`<br>`boost-process`<br>`curl`<br>`boost-property-tree`<br>`boost-asio`<br>`openssl`<br>`spdlog` |

## 开源许可证、重新分发

本程序使用 [GPLv3 许可证](https://github.com/richardchien/coolq-http-api/blob/master/LICENSE)，并按其第 7 节添加如下附加条款：

- 本程序的修改版本应以合理的方式标志为和原版本不同的版本（附加条款 c）

总体来说，在当前许可证下，你可以：

- 修改源代码并自己使用，在**不重新分发**（编译之后的程序）的情况下，没有任何限制
- 不修改源代码并重新分发，对程序收费或免费提供下载，或提供其它服务，此时你需要保证在明显的地方提供本程序的源码地址并保持协议不变（包括附加条款）
- 修改源代码并重新分发，对程序收费或免费提供下载，或提供其它服务，此时你需要注明源码修改的地方、提供源码地址、保持协议不变（可删除全部或部分附加条款）、修改程序的名称

## 问题、Bug 反馈、意见和建议

如果使用过程中遇到任何问题、Bug，或有其它意见或建议，欢迎提 [issue](https://github.com/richardchien/coolq-http-api/issues/new)。

也欢迎加入 QQ 交流群 201865589 来和大家讨论～

## 相似项目

- [Hstb1230/http-to-cq](https://github.com/Hstb1230/http-to-cq)
- [LEMOC](https://cqp.cc/t/29722)
- [yukixz/cqsocketapi](https://github.com/yukixz/cqsocketapi)

## 捐助

由于酷 Q 的一些功能只有 Pro 付费版才有，而我在编写插件时需要对每个可能的功能进行测试，我自己的使用中也没有对 Pro 版的需求，因此这将成为额外开销。如果你觉得本插件挺好用的，或对酷 Q Pro 的功能有需求，不妨进行捐助。你的捐助将用于开通酷 Q Pro 以测试功能，同时也会让我更加有动力完善插件。感谢你的支持！

[这里](https://github.com/richardchien/thanks) 列出了捐助者名单，由于一些收款渠道无法知道对方是谁，如有遗漏请联系我修改。

### 支付宝免费红包

![AliPay Red Bag](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/docs/alipay-redbag.jpg)

### 支付宝转账

![AliPay](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/docs/alipay.png)

### 微信转账

![WeChat](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/docs/wechat.png)
