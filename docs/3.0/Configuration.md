# 配置文件说明

配置文件使用 ini 格式编写，如下：

```ini
[general]
host=0.0.0.0
port=5700
post_url=http://192.168.0.11:8888
access_token=Mgep4rV49rM8Jf
```

注意上面的 `[general]` 不能少，它下方的配置是各账号的共用配置项。

如果需要对多个账号进行不同的配置，例如对不同的账号指定不同的上报地址和监听地址，则填写在 QQ 号对应的 section 下，如：

```ini
[general]
host=0.0.0.0

[12345678]
port=5700
post_url=http://192.168.0.11:8888
access_token=Mgep4rV49rM8Jf

[23456789]
host=127.0.0.1
port=5701
secret=kP9yK2lrGxoymmpo
```

**重要：如果配置文件中需要使用中文或其它非 ASCII 字符，则必须使用 UTF-8 without BOM 编码保存文件！**

支持的配置项如下：

| 配置项名称 | 默认值 | 说明 |
| -------- | ------ | --- |
| `host` | `0.0.0.0` | HTTP 服务器监听的 IP |
| `port` | `5700` | HTTP 服务器监听的端口 |
| `use_http` | `yes` | 是否开启 HTTP 接口，即通过 HTTP 调用 API |
| `ws_host` | `0.0.0.0` | WebSocket 服务器监听的 IP |
| `ws_port` | `6700` | WebSocket 服务器监听的端口 |
| `use_ws` | `no` | 是否开启 WebSocket 服务器，可用于调用 API 和推送事件，见 [WebSocket](/WebSocket) |
| `post_url` | 空 | 消息和事件的上报地址，通过 POST 方式请求，数据以 JSON 格式发送 |
| `access_token` | 空 | API 访问 token，如果不为空，则会在接收到请求时验证 `Authorization` 请求头是否为 `Token xxxxxxxx`，`xxxxxxxx` 为 access token |
| `secret` | 空 | 上报数据签名密钥，如果不为空，则会在 HTTP 上报时对 HTTP 正文进行 HMAC SHA1 哈希，使用 `secret` 的值作为密钥，计算出的哈希值放在上报的 `X-Signature` 请求头，例如 `X-Signature: sha1=f9ddd4863ace61e64f462d41ca311e3d2c1176e2` |
| `post_message_format` | `string` | 上报消息格式，`string` 为字符串格式，`array` 为数组格式，具体见 [消息格式](/Message) |
| `serve_data_files` | `no` | 是否提供请求 `data` 目录的文件的功能，`yes` 或 `true` 表示启用，否则不启用 |
| `update_source` | `https://raw.githubusercontent.com/richardchien/coolq-http-api-release/master/` | 更新源，默认使用 GitHub 的 [richardchien/coolq-http-api-release](https://github.com/richardchien/coolq-http-api-release) 仓库，对于酷 Q 运行在国内的情况，可以换成 `https://gitee.com/richardchien/coolq-http-api-release/raw/master/` |
| `update_channel` | `stable` | 更新通道，目前有 `stable` 和 `beta` 两个 |
| `auto_check_update` | `no` | 是否自动检查更新（每次启用插件时检查），`yes` 或 `true` 表示启用，否则不启用，不启用的情况下，仍然可以在酷 Q 应用菜单中手动检查更新 |
| `thread_pool_size` | `4` | 工作线程池大小，用于异步发送消息和一些其它小的异步任务，应根据计算机性能和实际需求适当调节 |
