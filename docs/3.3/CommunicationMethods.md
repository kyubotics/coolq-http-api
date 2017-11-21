# 通信方式

目前支持三种通信方式：

- 插件作为 HTTP 服务端，提供 API 和数据文件获取服务
- 插件作为 WebSocket 服务端，通过 `/api/` 和 `/event/` 两个接口分别提供 API 调用和事件推送服务
- 插件作为 WebSocket 客户端（称为「反向 WebSocket」），主动连接给定的 API 和事件上报地址，分别提供 API 调用服务和事件上报服务

上面三种通信方式分别使用 `use_http`、`use_ws`、`use_ws_reverse` 三个配置项来开关。

除了上述通信方式，还有通过 HTTP 上报事件，这是永远可用的，**不受上面三个 `use_*` 配置控制**，只要配置了 `post_url`，就会上报，并且处理响应数据。

下面详细介绍上面的三种通信方式的适用场景和使用方法。

## 插件作为 HTTP 服务端

### 适用场景

这是本插件最初支持的通信方式，也是使用起来最方便快捷的方式，适用于以下情况：

- 在本地初步测试使用酷 Q 和本插件，需要快速测试接口、查看接口返回的数据
- 运行酷 Q 的机器有公网 IP，或酷 Q 和业务代码运行在同一机器上
- 对于数据文件访问有需求
- ...

### 使用方法

将 `use_http` 配置为 `yes`（默认即 `yes`），然后通过 `host`、`port` 来配置要监听的 IP 和端口（默认为 `0.0.0.0:5700`），启用插件后即可通过形如 `http://host:port/send_private_msg?user_id=1234567&message=hello` 的 URL 来调用 API。

具体的 API 调用方法和 API 列表见 [API 描述](/API)。

## 插件作为 WebSocket 服务端

### 适用场景

- 运行酷 Q 的机器有公网 IP，或酷 Q 和业务代码运行在同一机器上
- 业务代码运行环境无法通过 HTTP 上报获得事件（例如浏览器中）
- ...

### 使用方法

将 `use_ws` 配置为 `yes`（默认 `no`），然后通过 `ws_host`、`ws_port` 来配置要监听的 IP 和端口（默认为 `0.0.0.0:6700`），启用插件后即可通过 `ws://ws_host:ws_port/api/` 接口来调用 API，通过 `ws://ws_host:ws_port/event/` 来接收事件推送。

这两个接口的具体用法见 [WebSocket API 描述](/WebSocketAPI)。

## 插件作为 WebSocket 客户端（反向 WebSocket）

### 适用场景

- 运行酷 Q 的机器没有公网 IP，且业务代码有公网 IP，或两者运行在同一机器上
- ...

### 使用方法

在业务代码中启动 WebSocket 服务端，开启两个接口，分别用于 API 调用和事件上报（如果只需要一个功能，也可以只开一个），然后分别配置 `ws_reverse_api_url`、`ws_reverse_event_url` 为上述两个接口的完整地址，例如 `ws://127.0.0.1:8765/api/`。再将 `use_ws_reverse` 配置为 `yes`（默认为 `no`），重启插件即可开启反向 WebSocket 服务。

插件会在特定的时候向指定的 URL 建立连接，如果配置了 `access_token`，则在建立连接时，会加入 `Authorization` 请求头，例如：

```http
Authorization: Token kSLuTF2GC2Q4q4ugm3
```

#### API 调用

首先插件启用时会启动一个**保持连接**的客户端用于连接 API 调用接口，即 `ws_reverse_api_url` 指定的接口，一旦收到服务端发来的消息就会调用相应的 API 并返回调用结果。

API 的调用方式和插件作为 WebSocket 服务端的 `/api/` 接口使用方式相同，见 [WebSocket API 描述的 `/api/`](/WebSocketAPI#api)，不同在于你的服务端必须在调用 API 后保持连接，以便下次调用。

#### 事件上报

插件启动时会启动一个**保持连接**的客户端用于连接事件上报接口，即 `ws_reverse_event_url` 指定的接口，在后续接收到酷 Q 的事件时，会通过这个连接发送事件数据。发送事件数据格式和 HTTP POST 方式上报的完全一致，见 [上报数据格式](/Post#上报数据格式)，事件列表见 [事件列表](/Post#事件列表)。

与 HTTP 上报不同的是，这里上报不会对数据进行签名（即 HTTP 上报中的 `X-Signature` 请求头在这里没有等价的东西），并且也不会处理响应数据。
