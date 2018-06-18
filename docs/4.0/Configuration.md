# 配置

配置文件支持 INI 和 JSON 两种格式，并支持单个文件配置多个账号，或每个账号对应一个配置文件。

插件启动时（或重启时）按如下顺序依次尝试加载配置文件，一旦有一条加载成功，则停止加载（所有路径均为相对于 `app\io.github.richardchien.coolqhttpapi` 的相对路径）：

- `config.(cfg|ini)`（扩展名的括号和竖线表示优先加载 `.cfg`，若没有，则加载 `.ini`，下同），文件中通用配置需要放在 `[general]` 下，QQ 号特定配置放在 `[<user_id>]` 下
- `config\general.(cfg|ini)` + `config\<user_id>.(cfg|ini)`，前者是通用配置，全部放在 `[general]` 下，后者是 QQ 号特定配置，全部放在 `[<user_id>]` 下，后者覆盖前者中已存在的内容
- `config.json`，通用配置需放在根对象的 `general` 字段，QQ 号特定配置放在 QQ 号对应字段，例如 `{"general": {"host": "0.0.0.0"}, "123456": {"port": 6666}}`
- `config\general.json` + `config\<user_id>.json`，配置项均直接放在根对象，后者覆盖前者中已存在的内容

**重要：如果配置文件中需要使用中文或其它非 ASCII 字符，则必须使用 UTF-8 without BOM 编码保存文件！**

## 几种典型的配置文件安排方式

### 使用 INI 格式，在单个文件中存放多个账号的配置

- `app\io.github.richardchien.coolqhttpapi\config.ini`

```ini
[general]
host = 0.0.0.0
post_url = http://192.168.0.11:8888

[12345678]
access_token = Mgep4rV49rM8Jf
port = 5700

[87654321]
port = 5701
```

### 使用 JSON 格式，在单个文件中存放多个账号的配置

- `app\io.github.richardchien.coolqhttpapi\config.json`

```json
{
    "general": {
        "host": "0.0.0.0",
        "post_url": "http://127.0.0.1:8080"
    },
    "12345678": {
        "access_token": "Mgep4rV49rM8Jf",
        "port": 5700
    },
    "87654321": {
        "port": 5701
    }
}
```

### 使用 JSON 格式，每个账号对应一个配置文件

- `app\io.github.richardchien.coolqhttpapi\config\general.json`

```json
{
    "host": "0.0.0.0",
    "post_url": "http://127.0.0.1:8080"
}
```

- `app\io.github.richardchien.coolqhttpapi\config\12345678.json`

```json
{
    "access_token": "Mgep4rV49rM8Jf",
    "port": 5700
}
```

- `app\io.github.richardchien.coolqhttpapi\config\87654321.json`

```json
{
    "port": 5701
}
```

## 配置项

| 配置项名称 | 默认值 | 说明 |
| -------- | ------ | --- |
| `host` | `0.0.0.0` | HTTP 服务器监听的 IP |
| `port` | `5700` | HTTP 服务器监听的端口 |
| `use_http` | `true` | 是否开启 HTTP 接口，即通过 HTTP 调用 API，见 [通信方式的第一种](/CommunicationMethods#插件作为-http-服务端) |
| `ws_host` | `0.0.0.0` | WebSocket 服务器监听的 IP |
| `ws_port` | `6700` | WebSocket 服务器监听的端口 |
| `use_ws` | `false` | 是否开启 WebSocket 服务器，可用于调用 API 和推送事件，见 [通信方式的第二种](/CommunicationMethods#插件作为-websocket-服务端) |
| `ws_reverse_api_url` | 空 | 反向 WebSocket API 地址 |
| `ws_reverse_event_url` | 空 | 反向 WebSocket 事件上报地址 |
| `ws_reverse_reconnect_interval` | `3000` | 反向 WebSocket 客户端断线重连间隔，单位毫秒 |
| `ws_reverse_reconnect_on_code_1000` | `false` | 是否在关闭状态码为 1000 的时候重连 |
| `use_ws_reverse` | `false` | 是否使用反向 WebSocket 服务，即插件作为 WebSocket 客户端主动连接指定的 API 和事件上报地址，见 [通信方式的第三种](/CommunicationMethods#插件作为-websocket-客户端（反向-websocket）) |
| `post_url` | 空 | 消息和事件的上报地址，通过 POST 方式请求，数据以 JSON 格式发送 |
| `access_token` | 空 | API 访问 token，如果不为空，则会在接收到请求时验证 `Authorization` 请求头是否为 `Token xxxxxxxx`，`xxxxxxxx` 为 access token |
| `secret` | 空 | 上报数据签名密钥，如果不为空，则会在 HTTP 上报时对 HTTP 正文进行 HMAC SHA1 哈希，使用 `secret` 的值作为密钥，计算出的哈希值放在上报的 `X-Signature` 请求头，例如 `X-Signature: sha1=f9ddd4863ace61e64f462d41ca311e3d2c1176e2` |
| `post_message_format` | `string` | 上报消息格式，`string` 为字符串格式，`array` 为数组格式，具体见 [消息格式](/Message) |
| `serve_data_files` | `false` | 是否提供请求 `data` 目录的文件的功能 |
| `update_source` | `https://raw.githubusercontent.com/richardchien/coolq-http-api-release/master/` | 更新源，默认使用 GitHub 的 [richardchien/coolq-http-api-release](https://github.com/richardchien/coolq-http-api-release) 仓库，对于酷 Q 运行在国内的情况，可以换成 `https://gitee.com/richardchien/coolq-http-api-release/raw/master/` |
| `update_channel` | `stable` | 更新通道，目前有 `stable`、`beta`、`alpha` 三个 |
| `auto_check_update` | `false` | 是否自动检查更新（每次启用插件时检查），不启用的情况下，仍然可以在酷 Q 应用菜单中手动检查更新 |
| `auto_perform_update` | `false` | 是否自动执行更新，仅在 `auto_check_update` 启用时有效，若启用，则插件将在自动检查到更新后，自动下载新版本并重启酷 Q 生效 |
| `thread_pool_size` | `4` | 工作线程池大小，用于异步发送消息和一些其它小的异步任务，应根据计算机性能和实际需求适当调节，若设为 0，则使用 `CPU 核心数 * 2 + 1` |
| `server_thread_pool_size` | `1` | API 服务器线程池大小，用于异步处理请求，应根据计算机性能和实际需求适当调节，若设为 0，则使用 `CPU 核心数 * 2 + 1` |
| `convert_unicode_emoji` | `true` | 是否在 CQ:emoji 和实际的 Unicode 之间进行转换，转换可能耗更多时间，但日常情况下影响不大，如果你的机器人需要处理非常大段的消息（上千字），且对性能有要求，可以考虑关闭转换 |
| `event_filter` | 空 | 指定事件过滤规则文件，见 [事件过滤器](/EventFilter)，留空将不开启事件过滤器 |
| `enable_backward_compatibility` | `false` | 是否启用旧版兼容性，启用时**事件上报**的数据将和 3.x 版本保持兼容 |
| `show_log_console` | `false` | 是否显示日志输出控制台 |
