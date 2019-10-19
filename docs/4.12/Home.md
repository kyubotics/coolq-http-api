# CoolQ HTTP API 插件

## 简介

通过 HTTP 对 酷Q 的事件进行上报以及接收 HTTP 请求来调用 酷Q 的 DLL 接口，从而可以使用其它语言编写 酷Q 插件。现已支持 WebSocket。

## 使用方法

### 手动安装

直接到 [Releases](https://github.com/richardchien/coolq-http-api/releases) 下载最新的 cpk 文件放到 酷Q 的 app 文件夹，然后启用即可。由于要上报事件、接受调用请求，因此需要所有权限。如果 Releases 里面下载不了，也可以去 [百度网盘](https://pan.baidu.com/s/1qY55zp6) 下载。

注意如果 酷Q 启动时报错说插件加载失败，或者系统弹窗提示缺少 DLL 文件，则需要安装 [Visual C++ 可再发行软件包](https://aka.ms/vs/16/release/vc_redist.x86.exe)（**点击链接即可从官方下载，如果自行安卓，一定要装 x86 也就是 32 位版本！**），如果你的系统是 Windows 7 或 Windows Server 2008、或者安装 Visual C++ 可再发行软件包之后仍然加载失败，则还需要安装 [通用 C 运行库更新](https://support.microsoft.com/zh-cn/help/3118401/update-for-universal-c-runtime-in-windows)，在这个链接里选择你系统对应的版本下载安装即可。如果此时还加载失败，请尝试重启系统。

启用后插件将开启一个 HTTP 服务器来接收请求，默认监听 `0.0.0.0:5700`，首次启用会生成一个默认配置文件，在 酷Q 的 `data\app\io.github.richardchien.coolqhttpapi\config` 文件夹中，文件名为 `<user_id>.json`（`<user_id>` 为登录的 QQ 号），使用 JSON 格式填写。关于配置项的说明，见 [配置](/Configuration)。

此时通过 `http://192.168.1.123:5700/` 即可调用 酷Q 的函数，例如 `http://192.168.1.123:5700/send_private_msg?user_id=123456&message=你好`，注意这里的 `192.168.1.123` 要换成你自己电脑的 IP，如果在本地跑，可以用 `127.0.0.1`，`user_id` 也要换成你想要发送到的 QQ 号。具体的 API 列表见 [API 描述](/API)。如果需要使用 HTTPS 来访问，见 [HTTPS](https://github.com/richardchien/coolq-http-api/wiki/HTTPS)。

酷Q 收到的消息、事件会被 POST 到配置文件中指定的 `post_url`，为空则不上报。上报数据格式见 [上报数据格式](/Post)。

停用插件将会关闭 HTTP 线程，再次启用将重新读取配置文件。

除了 HTTP 方式，现在还支持 WebSocket 和反向 WebSocket 两种通信方式，它们的适用场景和使用方法见 [通信方式](/CommunicationMethods)。

另外，本插件所支持的 CQ 码在 [原生 CQ 码](https://d.cqp.me/Pro/CQ%E7%A0%81) 的基础上进行了一些增强，见 [CQ 码](/CQCode)，并且支持以字符串或消息段数组格式表示消息，见 [消息格式](/Message)。

对于其它可能比较容易遇到的问题，见 [FAQ](https://github.com/richardchien/coolq-http-api/wiki/FAQ)。

### 使用 Docker

如果你使用 Docker 来部署服务，可以直接运行已制作好的 Docker 镜像，例如：

```bash
$ docker pull richardchien/cqhttp:latest
$ mkdir coolq  # 用于存储 酷Q 的程序文件
$ docker run -ti --rm --name cqhttp-test \
             -v $(pwd)/coolq:/home/user/coolq \  # 将宿主目录挂载到容器内用于持久化 酷Q 的程序文件
             -p 9000:9000 \  # noVNC 端口，用于从浏览器控制 酷Q
             -p 5700:5700 \  # HTTP API 插件开放的端口
             -e COOLQ_ACCOUNT=123456 \ # 要登录的 QQ 账号，可选但建议填
             -e CQHTTP_POST_URL=http://example.com:8080 \  # 事件上报地址
             -e CQHTTP_SERVE_DATA_FILES=yes \  # 允许通过 HTTP 接口访问 酷Q 数据文件
             richardchien/cqhttp:latest
```

其中，`CQHTTP_POST_URL`、`CQHTTP_SERVE_DATA_FILES` 是用于配置插件运行的，格式为「`CQHTTP_` + 插件配置项的大写」，具体的配置项，见 [配置](/Configuration)。

然后访问 `http://<你的IP>:9000/` 进入 noVNC（默认密码 `MAX8char`），登录 酷Q，即可开始使用（插件已自动启用，配置文件也根据启动命令的环境变量自动生成了）。一般情况下，你不太需要关注插件是如何存在于容器中的。

API 描述、事件上报等文档，见前面「手动安装」一节提供的指引链接。

关于在 Docker 中使用本插件的更多细节，见 [Docker](/Docker)。

## 从旧版升级

由于 4.x 版本引入了一些不兼容的变化，因此这里给出一份 [升级指南](/UpgradeGuide)，帮助用户从旧版升级到 4.x。
