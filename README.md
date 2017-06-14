# CoolQ HTTP API 插件

[![License](https://img.shields.io/badge/license-MIT%20License-blue.svg)](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/LICENSE)
[![Build Status](https://ci.appveyor.com/api/projects/status/9c4c6inmsdk3gbou?svg=true)](https://ci.appveyor.com/project/richardchien/coolq-http-api)
[![Release](https://img.shields.io/github/release/richardchien/coolq-http-api.svg)](https://github.com/richardchien/coolq-http-api/releases)
[![Download Count](https://img.shields.io/github/downloads/richardchien/coolq-http-api/total.svg)](https://github.com/richardchien/coolq-http-api/releases)
![QQ群](https://img.shields.io/badge/qq%E7%BE%A4-201865589-orange.svg)

通过 HTTP 对酷 Q 的事件进行上报以及接收 HTTP 请求来调用酷 Q 的 C++ 接口，从而可以使用其它语言编写酷 Q 插件。

## 使用方法

直接到 [Releases](https://github.com/richardchien/coolq-http-api/releases) 下载最新的 cpk 文件放到酷 Q 的 app 文件夹，然后启用即可。由于要上报事件、接受调用请求，因此需要所有权限。

注意如果系统中没有装 VC++ 2017 运行库，酷 Q 启动时会报错说插件加载失败，需要去下载 [Microsoft Visual C++ Redistributable for Visual Studio 2017](https://www.visualstudio.com/zh-hans/downloads/?q=redist) 安装。

启用后插件将开启一个后台线程用来监听 HTTP，默认监听 `0.0.0.0:5700`，首次启用会生成一个默认配置文件，在酷 Q app 文件夹的 `io.github.richardchien.coolqhttpapi` 文件夹中，文件名 `config.cfg`，使用 ini 格式填写。关于配置项的说明，见 [配置文件说明](https://richardchien.github.io/coolq-http-api/#/Configuration)。

此时通过 `http://192.168.1.123:5700/` 即可调用酷 Q 的函数，例如 `http://192.168.1.123:5700/send_private_msg?user_id=123456&message=你好`，注意这里的 `192.168.1.123` 要换成你自己电脑的 IP，如果在本地跑，可以用 `127.0.0.1`，`user_id` 也要换成你想要发送到的 QQ 号。具体的 API 列表见 [API 描述](https://richardchien.github.io/coolq-http-api/#/API)。

酷 Q 收到的消息、事件会被 POST 到配置文件中指定的 `post_url`，为空则不上报。上报数据格式见 [上报数据格式](https://richardchien.github.io/coolq-http-api/#/Post)。

停用插件将会关闭 HTTP 线程，再次启用将重新读取配置文件。

另外，本插件所支持的 CQ 码在原生的基础上进行了一些增强，见 [CQ 码](https://richardchien.github.io/coolq-http-api/#/CQCode)，并且支持以字符串或数组格式表示消息，见 [消息格式](https://richardchien.github.io/coolq-http-api/#/Message)。

## 文档

更多文档，见 [CoolQ HTTP API 插件文档](https://richardchien.github.io/coolq-http-api/)。

## 修改、编译

整个项目目录是一个 VS 2017 工程，使用了 VS 2017 (v141) 工具集，直接打开 `CoolQHttpApi.sln` 即可修改、编译。项目根目录也提供了 `build.bat` 脚本，可以直接 `cmd /k build.bat` 来编译，不过请注意查看脚本中的 VS 路径是否和你系统中的路径相符。

工程属性中配置了一个自定义的「后期生成事件」命令，也就是编译成功后执行的脚本，默认情况下什么都不做，你可以在项目根目录加一个文件 `post_build.bat` 在里面进行你需要的操作（比如移动 dll 文件到酷 Q 的 app 文件夹），这个文件被 Git 忽略掉了，因此不会影响 Git 仓库上的内容。

除了 `io.github.richardchien.coolqhttpapi.json`，其它所有文件均使用 UTF-8 编码，并且编译命令加入了 `/utf-8` 选项，因此如果你要修改代码，也请使用 UTF-8 编码，以便正常通过编译（注意，VS 创建文件之后输入中文可能会默认使用 GBK 编码，需要手动另存为到 UTF-8）。

`io.github.richardchien.coolqhttpapi.json` 文件中不应出现中文，如果因为一些原因必须加入中文，请使用 GBK 编码，否则酷 Q 无法识别。

## 问题、Bug 反馈、意见和建议

如果使用过程中遇到任何问题、Bug，或有其它意见或建议，欢迎提 [issue](https://github.com/richardchien/coolq-http-api/issues/new)。

也欢迎加入 QQ 交流群 201865589 来和大家讨论～

## 捐助

由于酷 Q 的一些功能只有 Pro 付费版才有，而我在编写插件时需要对每个可能的功能进行测试，我自己的使用中也没有对 Pro 版的需求，因此这将成为额外开销。如果你觉得本插件挺好用的，或对酷 Q Pro 的功能有需求，不妨进行捐助。你的捐助将用于开通酷 Q Pro 以测试功能，同时也会让我更加有动力完善插件。感谢你的支持！

[这里](https://github.com/richardchien/thanks) 列出了捐助者名单，由于一些收款渠道无法知道对方是谁，如有遗漏请联系我修改。

### 支付宝

![AliPay](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/docs/alipay.png)

### 微信支付

![WeChat](https://raw.githubusercontent.com/richardchien/coolq-http-api/master/docs/wechat.png)
