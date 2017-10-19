# CoolQ HTTP API 插件

## 简介

通过 HTTP 对酷 Q 的事件进行上报以及接收 HTTP 请求来调用酷 Q 的 C++ 接口，从而可以使用其它语言编写酷 Q 插件。

## 使用方法

### 手动安装

直接到 [Releases](https://github.com/richardchien/coolq-http-api/releases) 下载最新的 cpk 文件放到酷 Q 的 app 文件夹，然后启用即可。由于要上报事件、接受调用请求，因此需要所有权限。

注意如果系统中没有装 VC++ 2017 运行库，酷 Q 启动时会报错说插件加载失败，需要去下载 [Microsoft Visual C++ Redistributable for Visual Studio 2017 x86](https://www.visualstudio.com/zh-hans/downloads/?q=redist) 安装（注意一定要安装 x86 版本）。

启用后插件将开启一个后台线程用来监听 HTTP，默认监听 `0.0.0.0:5700`，首次启用会生成一个默认配置文件，在酷 Q app 文件夹的 `io.github.richardchien.coolqhttpapi` 文件夹中，文件名 `config.cfg`，使用 ini 格式填写。关于配置项的说明，见 [配置文件说明](/Configuration)。

此时通过 `http://192.168.1.123:5700/` 即可调用酷 Q 的函数，例如 `http://192.168.1.123:5700/send_private_msg?user_id=123456&message=你好`，注意这里的 `192.168.1.123` 要换成你自己电脑的 IP，如果在本地跑，可以用 `127.0.0.1`，`user_id` 也要换成你想要发送到的 QQ 号。具体的 API 列表见 [API 描述](/API)。如果需要使用 HTTPS 来访问，见 [HTTPS](https://github.com/richardchien/coolq-http-api/wiki/HTTPS)。

酷 Q 收到的消息、事件会被 POST 到配置文件中指定的 `post_url`，为空则不上报。上报数据格式见 [上报数据格式](/Post)。

停用插件将会关闭 HTTP 线程，再次启用将重新读取配置文件。

另外，本插件所支持的 CQ 码在原生的基础上进行了一些增强，见 [CQ 码](/CQCode)，并且支持以字符串或数组格式表示消息，见 [消息格式](/Message)。

对于其它可能比较容易遇到的问题，见 [FAQ](https://github.com/richardchien/coolq-http-api/wiki/FAQ)。

### 使用 Docker

如果你使用 docker 来部署服务，可以直接运行已制作好的 docker 镜像，容器将会按照环境变量的配置来下载或更新插件到指定或最新版本，并自动修改配置文件，例如：

```sh
$ docker pull richardchien/cqhttp:legacy
$ docker run -ti --rm --name cqhttp-test \
             -p 9000:9000 -p 5700:5700 \
             -e CQHTTP_VERSION=2.1.0 \
             -e CQHTTP_HOST=0.0.0.0 \
             -e CQHTTP_POST_URL=http://example.com:8080 \
             -e CQHTTP_SERVE_DATA_FILE=yes \
             richardchien/cqhttp:legacy
```

具体请参考 [richardchien/cqhttp-docker](https://github.com/richardchien/cqhttp-docker)。
