# 配置文件说明

默认配置填写在 `general` 中，即类似下面：

```ini
[general]
host=0.0.0.0
port=5700
post_url=http://192.168.0.11:8888
token=Mgep4rV49rM8Jf
```

如果需要登录多个账号并执行不同的逻辑，可以对不同的账号指定不同的上报地址和监听地址，如：

```ini
[general]
host=0.0.0.0
port=5700
post_url=
token=

[12345678]
post_url=http://192.168.0.11:8888
token=Mgep4rV49rM8Jf

[23456789]
port=5701
```

`[]` 中的也就是要配置的 QQ 号，其下方的配置会覆盖最上面 `general` 里的配置。注意 `general` 一定要放在最上方，因为最先读取它，然后继续读下面和当前登录号匹配的配置项。

支持的配置项如下：

| 配置项名称      | 默认值       | 说明                                       |
| ---------- | --------- | ---------------------------------------- |
| `host`     | `0.0.0.0` | HTTP 服务器监听的 IP                           |
| `port`     | `5700`    | HTTP 服务器监听的端口                            |
| `post_url` | 空         | 消息和事件的上报地址，通过 POST 方式请求，数据以 JSON 格式发送    |
| `token`    | 空         | 验证 token，如果不为空，则会在接收到请求是验证 `Authorization` 请求头是否为 `token xxxxxxxx`，`xxxxxxxx` 为 token，并在上报数据时在请求中加入同样的 `Authorization` 请求头。 |
