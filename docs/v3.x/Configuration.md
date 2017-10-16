# 配置文件说明

配置文件使用 ini 格式编写，配置项填写在要登录的 QQ 号为名的 section 中，即类似下面：

```ini
[12345678]
host=0.0.0.0
port=5700
post_url=http://192.168.0.11:8888
access_token=Mgep4rV49rM8Jf
```

支持对多个账号进行不同的配置，例如可以对不同的账号指定不同的上报地址和监听地址，如：

```ini
[12345678]
host=0.0.0.0
port=5700
post_url=http://192.168.0.11:8888
access_token=Mgep4rV49rM8Jf

[23456789]
host=0.0.0.0
port=5701
secret=kP9yK2lrGxoymmpo
```

**重要：如果配置文件中需要使用中文或其它非 ASCII 字符，则必须使用 UTF-8 without BOM 编码保存文件！**

支持的配置项如下：

| 配置项名称 | 默认值 | 说明 |
| -------- | ------ | --- |
| `host` | `0.0.0.0` | HTTP 服务器监听的 IP |
| `port` | `5700` | HTTP 服务器监听的端口 |
| `post_url` | 空 | 消息和事件的上报地址，通过 POST 方式请求，数据以 JSON 格式发送 |
| `post_timeout` | 20 | 消息、事件上报的超时时间（仅限制传输超时，而非连接超时），单位为秒，除非有特殊情况，通常无需修改 |
| `access_token` | 空 | API 访问 token，如果不为空，则会在接收到请求时验证 `Authorization` 请求头是否为 `Token xxxxxxxx`，`xxxxxxxx` 为 access token |
| `secret` | 空 | 上报数据签名密钥，如果不为空，则会在上报数据时对 HTTP 正文进行 HMAC SHA1 哈希，使用 `secret` 的值作为密钥，计算出的哈希值放在上报的 `X-Signature` 请求头，例如 `X-Signature: sha1=f9ddd4863ace61e64f462d41ca311e3d2c1176e2` |
| `post_message_format` | `string` | 上报消息格式，`string` 为字符串格式，`array` 为数组格式，具体见 [消息格式](https://richardchien.github.io/coolq-http-api/#/Message) |
| `serve_data_files` | `no` | 是否提供请求 `data` 目录的文件的功能，`yes` 或 `true` 表示启用，否则不启用 |
| `update_source` | `https://raw.githubusercontent.com/richardchien/coolq-http-api-release/master/` | 更新源，默认使用 GitHub 的 [richardchien/coolq-http-api-release] 仓库，对于酷 Q 运行在国内的情况，可以换成 `https://gitee.com/richardchien/coolq-http-api-release/raw/master/` |
| `update_channel` | `stable` | 更新通道，目前有 `stable` 和 `beta` 两个 |
| `auto_check_update` | `no` | 是否自动检查更新（每次启用插件时检查），`yes` 或 `true` 表示启用，否则不启用，不启用的情况下，仍然可以在酷 Q 应用菜单中手动检查更新 |
| `thread_pool_size` | `4` | 工作线程池大小，用于异步发送消息和一些其它小的异步任务，应根据计算机性能和实际需求适当调节 |
