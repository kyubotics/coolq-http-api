# 事件上报

## 上报方式

当配置文件中 `post_url` 配置项不为空时，会将酷 Q 收到的消息、事件通过 HTTP POST 上报，数据以 JSON 格式表示。

如果 `secret` 配置项也不为空，则会在每次上报的请求头中加入 HMAC 签名，如：

```http
POST / HTTP/1.1
X-Signature: sha1=f9ddd4863ace61e64f462d41ca311e3d2c1176e2
```

签名以 `secret` 作为密钥，HTTP 正文作为消息，进行 HMAC SHA1 哈希，你的后端可以通过该哈希值来验证上报的数据确实来自 HTTP API 插件。HMAC 介绍见 [密钥散列消息认证码](https://zh.wikipedia.org/zh-cn/%E9%87%91%E9%91%B0%E9%9B%9C%E6%B9%8A%E8%A8%8A%E6%81%AF%E9%91%91%E5%88%A5%E7%A2%BC)。

## 上报数据格式

每次上报的数据中必有的一个字段是 `post_type`，数据类型为字符串，用来指示此次上报的类型，有如下三种：

| 上报类型 | 说明 |
| ------- | --- |
| `message` | 收到消息 |
| `event` | 群、讨论组变动等非消息类事件 |
| `request` | 加好友请求、加群请求／邀请 |

其它字段随上报类型不同而有所不同，下面将在事件列表的「上报数据」标题下一一给出。

某些字段的值是一些固定的值，在表格的「可能的值」中给出，如果「可能的值」为空则表示没有固定的可能性。

另外，每一次上报都有一个 `time` 字段为事件发生的时间戳，数据类型为数字，下面不再列出。

## 上报请求的响应数据格式

从 1.1.0 版本开始支持在上报请求的响应中直接指定一些简单的操作。

如果不需要使用这个特性，返回 HTTP 响应状态码 204，或保持响应正文内容为空；如果需要，则使用 JSON 作为响应正文，`Content-Type` 响应头任意（目前不会进行判断），但设置为 `application/json` 最好，以便减少不必要的升级成本，因为如果以后有需求，可能会加入判断。

响应的 JSON 数据中，支持的操作随事件的不同而不同，会在后面的事件列表中的「响应数据」标题下一一给出。需要指出的是，**响应数据中的每个字段都是可选的**，只有在字段存在（明确要求进行操作）时，才会触发相应的操作，否则将保持对酷 Q 整体运行状态影响最小的行为（比如默认不拦截事件、不回复消息、不处理请求）。

「响应数据」给出的表格中的「允许的值」表示该字段允许的数据值，如果你返回的值不在允许的值范围内，则会采用默认行为，「允许的值」为空表示不限制。

每个事件都有一个共同的支持的操作，即 `block` 字段，数据类型为 boolean，`true` 表示拦截事件（不再让后面的插件处理），否则表示忽略（不拦截），例如：

```json
{
    "block": true
}
```

如果你在处理上报请求时，返回上面的 JSON 作为响应，则优先级在酷 Q 之后的其它插件将不会再收到这个事件。

此 `block` 字段后面不再列出。

另外，`post_type` 为 `event` 的上报请求，都只支持响应 `block` 字段，而不支持其它操作。

## 上报和回复消息格式

从 2.0.0 版本开始上报和回复消息格式有了较大变化（向下兼容），具体请查看 [消息格式](/Message)。

本页后面提到消息的地方（消息事件的 `message` 字段和响应的 `reply` 字段），将不再具体解释其格式。

## 事件列表

### 私聊消息

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------- | ------- | ---- |
| `post_type` | string | `"message"` | 上报类型 |
| `message_type` | string | `"private"` | 消息类型 |
| `sub_type` | string | `"friend"`、`"group"`、`"discuss"`、`"other"` | 消息子类型，如果是好友则是 `"friend"`，如果从群或讨论组来的临时会话则分别是 `"group"`、`"discuss"` |
| `user_id` | number | - | 发送者 QQ 号 |
| `message` | string/array | - | 消息内容 |
| `font` | number | - | 字体 |

#### 响应数据

| 字段名 | 数据类型 | 允许的值 | 说明 |
| ----- | ------- | ------ | ---- |
| `reply` | string/array | - | 要回复的内容 |
| `auto_escape` | boolean | `true`, `false` | 消息内容是否作为纯文本发送（即不解析 CQ 码），`message` 数据类型为 `array` 时无效 |

### 群消息

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------- | ------- | --- |
| `post_type` | string | `"message"` | 上报类型 |
| `message_type` | string | `"group"` | 消息类型 |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 发送者 QQ 号 |
| `anonymous` | string | - | 匿名用户显示名 |
| `anonymous_flag` | string | - | 匿名用户 flag，在调用禁言 API 时需要传入 |
| `message` | string/array | - | 消息内容 |
| `font` | number | - | 字体 |

#### 响应数据

| 字段名 | 数据类型 | 允许的值 | 说明 |
| ----- | ------- | ------- | --- |
| `reply` | string/array | - | 要回复的内容 |
| `auto_escape` | boolean | `true`, `false` | 消息内容是否作为纯文本发送（即不解析 CQ 码），`message` 数据类型为 `array` 时无效 |
| `at_sender` | boolean | `true`, `false` | 是否要在回复开头 at 发送者（自动添加），默认为 `true`，发送者是匿名用户时无效 |
| `kick` | boolean | `true`, `false` | 把发送者踢出群组（需要登录号权限足够），**不拒绝**此人后续加群请求，默认为 `false`，发送者是匿名用户时无效 |
| `ban` | boolean | `true`, `false` | 把发送者禁言 30 分钟（需要登录号权限足够），对匿名用户也有效，不支持指定禁言时长（如需指定，请调用相应 API），默认为 `false` |

### 讨论组消息

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------- | ------- | --- |
| `post_type` | string | `"message"` | 上报类型 |
| `message_type` | string | `"discuss"` | 消息类型 |
| `discuss_id` | number | - | 讨论组 ID |
| `user_id` | number | - | 发送者 QQ 号 |
| `message` | string/array | - | 消息内容 |
| `font` | number | - | 字体 |

#### 响应数据

| 字段名 | 数据类型 | 允许的值 | 说明 |
| ----- | ------- | ------- | --- |
| `reply` | string/array  | - | 要回复的内容 |
| `auto_escape` | boolean | `true`, `false` | 消息内容是否作为纯文本发送（即不解析 CQ 码），`message` 数据类型为 `array` 时无效 |
| `at_sender` | boolean | `true`, `false` | 是否要在回复开头 at 发送者（自动添加），默认为 `true` |

### 群文件上传

**注意：仅群文件上传表现为事件，好友发送文件在酷 Q 中没有独立的事件，而是直接表现为好友消息，请注意在编写业务逻辑时进行判断。**

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------ | ------- | ---- |
| `post_type` | string | `"event"` | 上报类型 |
| `event` | string | `"group_upload"` | 事件名 |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 发送者 QQ 号 |
| `file` | object | - | 文件信息 |

其中 `file` 字段的内容如下：

| 字段名 | 数据类型 | 说明 |
| ----- | ------ | ---- |
| `id` | string | 文件 ID |
| `name` | string | 文件名 |
| `size` | number | 文件大小（字节数） |
| `busid` | number | busid（目前不清楚有什么作用） |

### 群管理员变动

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------ | -------- | --- |
| `post_type` | string | `"event"` | 上报类型 |
| `event` | string | `"group_admin"` | 事件名 |
| `sub_type` | string | `"set"`、`"unset"` | 事件子类型，分别表示设置和取消管理员 |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 管理员 QQ 号 |

### 群成员减少

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------ | -------- | --- |
| `post_type` | string | `"event"` | 上报类型 |
| `event` | string | `"group_decrease"` | 事件名 |
| `sub_type` | string | `"leave"`、`"kick"`、`"kick_me"` | 事件子类型，分别表示主动退群、成员被踢、登录号被踢 |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 离开者 QQ 号 |
| `operator_id` | number | - | 操作者 QQ 号（如果是主动退群，则和 `user_id` 相同） |

### 群成员增加

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------ | -------- | --- |
| `post_type` | string | `"event"` | 上报类型 |
| `event` | string | `"group_increase"` | 事件名 |
| `sub_type` | string | `"approve"`、`"invite"` | 事件子类型，分别表示管理员已同意入群、管理员邀请入群 |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 加入者 QQ 号 |
| `operator_id` | number | - | 操作者 QQ 号 |

### 好友添加

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------ | -------- | --- |
| `post_type` | string | `"event"` | 上报类型 |
| `event` | string | `"friend_add"` | 事件名 |
| `user_id` | number | - | 新添加好友 QQ 号 |

### 加好友请求

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------ | -------- | --- |
| `post_type` | string | `"request"` | 上报类型 |
| `request_type` | string | `"friend"`  | 请求类型 |
| `user_id` | number | - | 发送请求的 QQ 号 |
| `message` | string | - | 验证信息 |
| `flag` | string | - | 请求 flag，在调用处理请求的 API 时需要传入 |

#### 响应数据

| 字段名 | 数据类型 | 允许的值 | 说明 |
| ----- | ------- | ------- | --- |
| `approve` | boolean | `true`, `false` | 是否同意请求 |
| `remark` | string  | - | 添加后的好友备注（仅在同意时有效） |

### 加群请求／邀请

#### 上报数据

| 字段名 | 数据类型 | 可能的值 | 说明 |
| ----- | ------ | -------- | --- |
| `post_type` | string | `"request"` | 上报类型 |
| `request_type` | string | `"group"` | 请求类型 |
| `sub_type` | string | `"add"`、`"invite"` | 请求子类型，分别表示加群请求、邀请登录号入群 |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 发送请求的 QQ 号 |
| `message` | string | - | 验证信息 |
| `flag` | string | - | 请求 flag，在调用处理请求的 API 时需要传入 |

#### 响应数据

| 字段名 | 数据类型 | 允许的值 | 说明 |
| ----- | ------- | ------- | --- |
| `approve` | boolean | `true`, `false` | 是否同意请求／邀请 |
| `reason` | string | - | 拒绝理由（仅在拒绝时有效） |
