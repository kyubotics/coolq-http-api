# 配置文件说明

配置项全部填写在 `general` 中，即类似下面：

```ini
[general]
host=0.0.0.0
port=5700
post_url=http://192.168.0.11:8888
token=Mgep4rV49rM8Jf
```

配置项如下：

| 配置项名称      | 默认值       | 说明                                       |
| ---------- | --------- | ---------------------------------------- |
| `host`     | `0.0.0.0` | HTTP 服务器监听的 IP                           |
| `port`     | `5700`    | HTTP 服务器监听的端口                            |
| `post_url` | 空         | 消息和事件的上报地址，通过 POST 方式请求，数据以 JSON 格式发送    |
| `token`    | 空         | 验证 token，如果不为空，则会在接收到请求是验证 `Authorization` 请求头是否为 `token xxxxxxxx`，`xxxxxxxx` 为 token，并在上报数据时在请求中加入同样的 `Authorization` 请求头。 |
