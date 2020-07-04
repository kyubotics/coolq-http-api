# CQHTTP

[![License](https://img.shields.io/github/license/richardchien/coolq-http-api.svg)](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/LICENSE)
[![Build Status](https://img.shields.io/appveyor/ci/richardchien/coolq-http-api.svg)](https://ci.appveyor.com/project/richardchien/coolq-http-api)
[![Release](https://img.shields.io/github/release/richardchien/coolq-http-api.svg)](https://github.com/richardchien/coolq-http-api/releases)
[![Download Count](https://img.shields.io/github/downloads/richardchien/coolq-http-api/total.svg)](https://github.com/richardchien/coolq-http-api/releases)
[![Docker Build Status](https://img.shields.io/travis/richardchien/coolq-http-api.svg?label=docker%20build)](https://travis-ci.org/richardchien/coolq-http-api)
[![Docker Pulls](https://img.shields.io/docker/pulls/richardchien/cqhttp.svg)](https://hub.docker.com/r/richardchien/cqhttp/)
[![QQ 群](https://img.shields.io/badge/qq%E7%BE%A4-201865589-orange.svg)](https://jq.qq.com/?_wv=1027&k=5Euplde)
[![Telegram](https://img.shields.io/badge/telegram-chat-blue.svg)](https://t.me/cqhttp)
[![QQ 版本发布群](https://img.shields.io/badge/%E7%89%88%E6%9C%AC%E5%8F%91%E5%B8%83%E7%BE%A4-218529254-green.svg)](https://jq.qq.com/?_wv=1027&k=5Nl0zhE)
[![Telegram 版本发布频道](https://img.shields.io/badge/%E7%89%88%E6%9C%AC%E5%8F%91%E5%B8%83%E9%A2%91%E9%81%93-join-green.svg)](https://t.me/cqhttp_release)

通过 HTTP 或 WebSocket 对 [酷Q](https://cqp.cc/) 的事件进行上报以及接收请求来调用 酷Q 的 DLL 接口，从而可以使用其它语言编写 酷Q 插件。支持 Windows 7 和 Windows Server 2008 及更新版本，也可以运行在 Wine、Docker。

## 使用方法

使用方法见 [https://cqhttp.cc/docs/](https://cqhttp.cc/docs/)。v1.x、v2.x 的旧版本文档在 [https://cqhttp.cc/docs/legacy/](https://cqhttp.cc/docs/legacy/)。

如果访问这个文档非常慢，请尝试国内的地址 [http://richardchien.gitee.io/coolq-http-api/docs/](http://richardchien.gitee.io/coolq-http-api/docs/)。

## SDK／开发框架

对于下面这些语言的开发者，如果不想自己处理繁杂的请求和解析操作，可以尝试社区中开发者们已经封装好的的 SDK 或开发框架：

| | 语言 | Web 框架 | 通信方式 | 地址 | 核心作者 |
| --- | --- | --- | --- | --- | --- |
| ⭐ | Python | Quart | HTTP,<br>反向 WebSocket | [nonebot/nonebot](https://github.com/nonebot/nonebot) | richardchien<br>yanyongyu |
| ⭐ | Python | Quart | HTTP,<br>反向 WebSocket | [nonebot/aiocqhttp](https://github.com/nonebot/aiocqhttp) | richardchien |
| ⭐ | Node.js | WebSocket-Node | WebSocket | [momocow/node-cq-websocket](https://github.com/momocow/node-cq-websocket) | momocow |
| ⭐ | Node.js | Express | HTTP,<br>WebSocket | [koishijs/koishi](https://github.com/koishijs/koishi) | Shigma |
|  | Node.js | WebSocket-Node | WebSocket | [CaoMeiYouRen/node-coolq-robot](https://github.com/CaoMeiYouRen/node-coolq-robot) | CaoMeiYouRen |
|  | Node.js | Express | HTTP | [XHMM/lemon-bot](https://github.com/XHMM/lemon-bot) | XHMM |
|  | JavaScript | - | WebSocket | [pandolia/js-bot](https://github.com/pandolia/js-bot) | pandolia |
|  | Deno | 标准库 | 反向 WebSocket | [nenojs/deno-cqhttp](https://github.com/nenojs/deno-cqhttp) | rikakomoe |
| ⭐ | PHP | Swoole | 反向 WebSocket | [zhamao-robot/zhamao-framework](https://github.com/zhamao-robot/zhamao-framework) | crazywhalecc |
|  | PHP | - | HTTP | [LovelyA72/YeziiBot-v2](https://github.com/LovelyA72/YeziiBot-v2) | LovelyA72 |
| ⭐ | Java | 标准库 | HTTP | [HyDevelop/PicqBotX](https://github.com/HyDevelop/PicqBotX) | Hykilpikonna |
|  | Java<br>Kotlin<br>Groovy | Spring | 反向 WebSocket | [lz1998/Spring-CQ](https://github.com/lz1998/Spring-CQ)（[教程](https://www.bilibili.com/video/av89649630/)） | lz1998 |
|  | Java | 标准库 | HTTP | [ForteScarlet/simple-robot-core](https://github.com/ForteScarlet/simple-robot-core) | ForteScarlet |
|  | Java | JFinal | HTTP | [thevsk/cqhttp-java-jfinal-sdk](https://github.com/thevsk/cqhttp-java-jfinal-sdk) | thevsk |
|  | Kotlin | 标准库 | HTTP | [JuerGenie/juerobot](https://github.com/JuerGenie/juerobot) | JuerGenie |
| ⭐ | Go | 标准库 | **API:**<br>HTTP,<br>WebSocket<br>**Event:**<br>HTTP,<br>长轮询,<br>WebSocket,<br>反向 WebSocket | [catsworld/qq-bot-api](https://github.com/catsworld/qq-bot-api) | catsworld<br>rikakomoe |
| ⭐ | C# | 标准库 | HTTP,<br>WebSocket | [int-and-his-friends/Sisters.WudiLib](https://github.com/int-and-his-friends/Sisters.WudiLib) | bleatingsheep |
| ⭐ | C# | 标准库 | HTTP,<br>WebSocket,<br>反向 WebSocket | [frank-bots/cqhttp.Cyan](https://github.com/frank-bots/cqhttp.Cyan) | frankli0324 |
|  | C# | Fleck | 反向 WebSocket | [cqbef/cqhttp.WebSocketReverse.NETCore](https://github.com/cqbef/cqhttp.WebSocketReverse.NETCore) | cqbef |
|  | PowerShell | .NET | HTTP | [cqmoe/cqhttp-powershell-sdk](https://github.com/cqmoe/cqhttp-powershell-sdk) | richardchien |
|  | Lua | lua-http | HTTP,<br>WebSocket | [cleoold/cqhttp-lua53-sdk](https://github.com/cleoold/cqhttp-lua53-sdk) | cleoold |

## 应用案例

QQ 机器人可以用来做很多有意思的事情，下面列出一些基于本插件的应用案例：

| 项目地址 | 简介 |
| ------- | --- |
| [milkice233/efb-qq-slave](https://github.com/milkice233/efb-qq-slave) | 基于 ehForwarderBot 框架的 QQ 从端 |
| [projectriri/bot-gateway](https://projectriri.github.io/bot-gateway/) | 提供跨聊天平台的通用机器人 API 的机器人消息网关 |
| [jqqqqqqqqqq/UnifiedMessageRelay](https://github.com/jqqqqqqqqqq/UnifiedMessageRelay) | QQ <-> Telegram Bot Framework & Forwarder |
| [Mother-Ship/cabbageWeb](https://github.com/Mother-Ship/cabbageWeb) | 基于 Java Web 的 osu! 游戏数据查询机器人 |
| [bdbai/Kobirt](https://github.com/bdbai/Kobirt) | Ingress 游戏辅助机器人 |
| [JRT-FOREVER/hub2coolq](https://github.com/JRT-FOREVER/hub2coolq) | GitHub webhook 消息转发至 QQ 群 |
| [Kurarion/Bangumi-for-QQ](https://github.com/Kurarion/Bangumi-for-QQ) | 用于在 QQ 平台便捷地使用 Bangumi 部分功能（查寻条目、更新条目进度等） |
| [rikakomoe/cqhttp-twitter-bot](https://github.com/rikakomoe/cqhttp-twitter-bot) | 自动订阅 Twitter 发送到 QQ |
| [XiaoLin0815/QQ2TG](https://github.com/XiaoLin0815/QQ2TG) | 帮助 QQ 与 Telegram 互联的小程序 |
| [spacemeowx2/splatoon2-qqbot](https://github.com/spacemeowx2/splatoon2-qqbot) | 宇宙第一的 Splatoon2 的地图机器人 |
| [OYMiss/forward-bot](https://github.com/OYMiss/forward-bot) | 用 Telegram 和 QQ 好友聊天的转发机器人 |
| [mrthanlon/SICNUBOT](https://github.com/mrthanlon/SICNUBOT) | 专为四川师范大学设计用于审核发布消息用的 QQ 机器人 |
| [billjyc/pocket48](https://github.com/billjyc/pocket48/tree/coolq) | 监控成员口袋 48 聚聚房间、微博和摩点项目 |
| [chinshin/CQBot_hzx](https://github.com/chinshin/CQBot_hzx) | 摩点 & 微博 & 口袋 48 机器人（BEJ48-黄子璇） |
| [Ice-Hazymoon/grnd_bot](https://github.com/Ice-Hazymoon/grnd_bot) | 订阅 RSSHub 更新并推送到 QQ 群 |
| [Ray-Eldath/Avalon](https://github.com/Ray-Eldath/Avalon) | 多功能、可扩展的群机器人，支持 QQ 和 Discord |
| [Bluefissure/FFXIVBOT](https://github.com/Bluefissure/FFXIVBOT) | 基于 Django Channels 的最终幻想 14 游戏数据查询机器人 |
| [Milkitic/Daylily](https://github.com/Milkitic/Daylily) | 基于 ASP.NET Core 的跨平台机器人（含快速开发插件框架） |
| [cczu-osa/aki](https://github.com/cczu-osa/aki) | 基于 NoneBot 的多功能 QQ 机器人 |
| [cleoold/sendo-erika](https://github.com/cleoold/sendo-erika) | 基于 cqhttp 和 NoneBot 的，主要通过私聊摇控的 QQ 机器人 |
| [duan602728596/qqtools](https://github.com/duan602728596/qqtools) | 基于 Nwjs 的 QQ 群工具（摩点、口袋 48、微博提醒、入群欢迎、定时喊话、自定义命令和回复信息等） |
| [Tsuk1ko/CQ-picfinder-robot](https://github.com/Tsuk1ko/CQ-picfinder-robot) | 基于 Saucenao 的搜图机器人 |
| [kasora/dice](https://github.com/kasora/dice) | COC7 骰子 QQ 机器人 |
| [shidenggui/tuishujun-for-qq](https://github.com/shidenggui/tuishujun-for-qq) | 基于推书君的小说查询推荐 QQ 机器人 |
| [JuerGenie/cn.juerwhang.jgbot](https://github.com/JuerGenie/cn.juerwhang.jgbot) | 基于 [JuerGenie/juerobot](https://github.com/JuerGenie/juerobot) 的娱乐用 QQ 机器人 |
| [drsanwujiang/DiceRobot](https://github.com/drsanwujiang/DiceRobot) | 一个基于 coolq-http-api 插件的 TRPG 骰子机器人 |
| [UltraSoundX/SDFMU-Library](https://github.com/UltraSoundX/SDFMU-Library) | 山东第一医科大图书馆预约机器人 |
| [Quan666/ELF_RSS](https://github.com/Quan666/ELF_RSS) | 基于 NoneBot 的，交互式 RSS 订阅、转发机器人 |
| [suisei-cn/stargazer-qq](https://github.com/suisei-cn/stargazer-qq) | 一个灵活的 vtuber 发推/直播动态监控机器人 |
| [Ninzore/Wecab](https://github.com/Ninzore/Wecab) | 网络内容聚合机器人，支持微博、B站、Twitter 等 |

以上列出的只是实际应用中的一小部分，如果你使用本插件编写了任何好的应用案例或开发框架，也可以通过 issue 或 pull request 添加到这里的列表，共同充实社区。

## Nightly 版本

如果你急需测试尚未发布的最新特性，可以前往 https://ci.appveyor.com/project/richardchien/coolq-http-api/build/artifacts 下载从 master 分支的最新代码构建出的 DLL 和 JSON 文件，将它们放入 酷Q 的 `app` 文件夹并开启 酷Q 的开发模式即可使用。

**需要注意的是，master 分支所构建出的插件并不确保任何时候都是可用的，可能出现功能存在，但尚未完成的情况。**

## 修改、编译

本项目基于 [CoolQ C++ SDK](https://github.com/richardchien/coolq-cpp-sdk)，构建方式和它一致，但由于一些历史原因，需要先手动下载预编译的依赖库，并且使用 **Visual Studio 2019** 和 **MSVC v141** 构建（可以只安装 Build Tools，不必安装完整的 IDE）。

首先，克隆或下载本仓库代码：

```ps1
git clone https://github.com/richardchien/coolq-http-api.git
cd coolq-http-api
git submodule init
git submodule update
```

然后，从 [这里](https://richardchien-my.sharepoint.com/:u:/g/personal/i_page_moe/Edh080tNqhZGvB7Qb3CR8k4BLdqEwdWZpmJJmig6qs9bLg?e=5wK22p) 下载 `vcpkg-export-20191012.zip`（也就是预编译的依赖），并解压到当前工程目录，确保工程目录结构如下：

```
coolq-http-api
├── ...
├── src
├── vcpkg-export-20191012
|   ├── installed
|   ├── scripts
│   └── .vcpkg-root
├── CMakeLists.txt
└── ...
```

最后生成并构建 CMake 项目：

```ps1
powershell .\scripts\generate.ps1 Debug
powershell .\scripts\build.ps1 Debug
```

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

## 感谢

- 感谢 酷Q 项目，为本插件的存在提供了可能
- 感谢所有 SDK 作者，扩展了本插件的生态，使用户更容易上手
- 感谢所有捐助者对我的鼓励，[这里](https://github.com/richardchien/thanks) 列出了捐助者名单（由于一些收款渠道无法知道对方是谁，如有遗漏请联系我修改）
- 感谢 [sjdy521/Mojo-Webqq](https://github.com/sjdy521/Mojo-Webqq) 项目给本插件的接口设计提供了参考
- 感谢所有用户反馈的 bug 和建议，使本插件不断完善

## 相似项目

除了 CQHTTP，还有一些其它相似功能的插件：

- [Hstb1230/http-to-cq](https://github.com/Hstb1230/http-to-cq)
- [LEMOC](https://cqp.cc/t/29722)
- [yukixz/cqsocketapi](https://github.com/yukixz/cqsocketapi)

除了上述基于 酷Q 平台的插件，还有一些基于其它机器人平台、旨在兼容 CQHTTP 接口的项目，为 QQ 机器人开发社区注入了新的活力：

- [yyuueexxiinngg/cqhttp-mirai](https://github.com/yyuueexxiinngg/cqhttp-mirai)（兼容部分 API）
- [iTXTech/mirai-native](https://github.com/iTXTech/mirai-native)（通过直接加载 CQHTTP 的 DLL 实现）

## 捐助

本项目是我在业余时间开发并免费提供使用的，如果你觉得本插件挺好用，不妨进行捐助，别忘了备注你的 ID 或昵称，以便我添加到捐助者名单～

你的捐助会让我更加有动力完善插件，感谢你的支持！

### 爱发电

https://afdian.net/@richardchien

### 微信赞赏码

![WeChat Donate](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/docs/wechat-donate.png)

### 支付宝转账

![AliPay](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/docs/alipay.png)

### PayPal

https://paypal.me/richardchien0
