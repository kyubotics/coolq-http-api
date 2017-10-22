# CoolQ HTTP API 插件

## 简介

通过 HTTP 对酷 Q 的事件进行上报以及接收 HTTP 请求来调用酷 Q 的 DLL 接口，从而可以使用其它语言编写酷 Q 插件。现已支持 WebSocket。

## 使用方法

### 手动安装

直接到 [Releases](https://github.com/richardchien/coolq-http-api/releases) 下载最新的 cpk 文件放到酷 Q 的 app 文件夹，然后启用即可。由于要上报事件、接受调用请求，因此需要所有权限。如果 Releases 里面下载不了，也可以去 [百度网盘](https://pan.baidu.com/s/1qY55zp6) 下载。

注意如果系统中没有装 VC++ 2017 运行库，酷 Q 启动时会报错说插件加载失败，需要去下载 [Microsoft Visual C++ Redistributable for Visual Studio 2017 x86](https://www.visualstudio.com/zh-hans/downloads/?q=redist) 安装，**注意一定要安装 x86 版本**！

启用后插件将开启一个 HTTP 服务器来接收请求，默认监听 `0.0.0.0:5700`，首次启用会生成一个默认配置文件，在酷 Q app 文件夹的 `io.github.richardchien.coolqhttpapi` 文件夹中，文件名 `config.cfg`，使用 ini 格式填写。关于配置项的说明，见 [配置文件说明](/Configuration)。

此时通过 `http://192.168.1.123:5700/` 即可调用酷 Q 的函数，例如 `http://192.168.1.123:5700/send_private_msg?user_id=123456&message=你好`，注意这里的 `192.168.1.123` 要换成你自己电脑的 IP，如果在本地跑，可以用 `127.0.0.1`，`user_id` 也要换成你想要发送到的 QQ 号。具体的 API 列表见 [API 描述](/API)。如果需要使用 HTTPS 来访问，见 [HTTPS](https://github.com/richardchien/coolq-http-api/wiki/HTTPS)。

酷 Q 收到的消息、事件会被 POST 到配置文件中指定的 `post_url`，为空则不上报。上报数据格式见 [上报数据格式](/Post)。

停用插件将会关闭 HTTP 线程，再次启用将重新读取配置文件。

除了 HTTP 方式，现也支持通过 WebSocket 调用接口和接收事件，见 [WebSocket](/WebSocket)。

另外，本插件所支持的 CQ 码在原生的基础上进行了一些增强，见 [CQ 码](/CQCode)，并且支持以字符串或数组格式表示消息，见 [消息格式](/Message)。

对于其它可能比较容易遇到的问题，见 [FAQ](https://github.com/richardchien/coolq-http-api/wiki/FAQ)。

### 使用 Docker

如果你使用 docker 来部署服务，可以直接运行已制作好的 docker 镜像，例如：

```bash
$ docker pull richardchien/cqhttp:latest
$ mkdir coolq  # 用于存储酷 Q 的程序文件
$ docker run -ti --rm --name cqhttp-test \
             -v $(pwd)/coolq:/home/user/coolq \  # 将宿主目录挂载到容器内用于持久化酷 Q 的程序文件
             -p 9000:9000 \  # noVNC 端口，用于从浏览器控制酷 Q
             -p 5700:5700 \  # HTTP API 插件开放的端口
             -e CQHTTP_POST_URL=http://example.com:8080 \  # 事件上报地址
             -e CQHTTP_SERVE_DATA_FILES=yes \  # 允许通过 HTTP 接口访问酷 Q 数据文件
             richardchien/cqhttp:latest
```

其中，`CQHTTP_POST_URL`、`CQHTTP_SERVE_DATA_FILES` 是用于配置插件运行的，格式为「`CQHTTP_` + 插件配置项的大写」，具体的配置项，见 [配置文件说明](/Configuration)。

然后访问 `http://<你的IP>:9000/` 进入 noVNC（默认密码 `MAX8char`），登录酷 Q，即可开始使用（插件已自动启用，配置文件也根据启动命令的环境变量自动生成了）。一般情况下，你不太需要关注插件是如何存在于容器中的。

API 描述、事件上报等文档，见前面「手动安装」一节提供的指引链接。

关于在 docker 中使用本插件的更多细节，见 [Docker](/Docker)。

## 从旧版升级

由于 3.0 版本引入了一些 breaking change，因此这里给出一份 [升级指南](/UpgradeGuide)，帮助用户从旧版升级到 3.0。
