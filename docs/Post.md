# 上报数据格式

## 上报方式

当配置文件中 `post_url` 配置项不为空时，会将酷 Q 收到的消息、事件通过 HTTP POST 上报，数据以 JSON 格式表示。

如果 `token` 配置项也不为空，则会在每次上报的请求头中加入验证头，如：

```http
POST / HTTP/1.1
Authorization: token kSLuTF2GC2Q4q4ugm3
```

`kSLuTF2GC2Q4q4ugm3` 换成你填写的 token。

## 数据格式

每次上报的数据中必有的一个字段是 `post_type`，数据类型为字符串，用来指示此次上报的类型，有如下三种：

| 上报类型      | 说明             |
| --------- | -------------- |
| `message` | 收到消息           |
| `event`   | 群、讨论组变动等非消息类事件 |
| `request` | 加好友请求、加群请求／邀请  |

其它字段随上报类型不同而有所不同，下面将一一给出。

某些字段的值是一些固定的值，在「可能的值」中给出，如果「可能的值」为空则表示没有固定的可能性。

另外，每一次上报都有一个 `time` 字段为事件发生的时间戳，数据类型为数字，下面不在列出。

## 事件列表

### 私聊消息

| 字段名            | 数据类型   | 可能的值                                     | 说明                                       |
| -------------- | ------ | ---------------------------------------- | ---------------------------------------- |
| `post_type`    | string | `"message"`                              | 上报类型                                     |
| `message_type` | string | `"private"`                              | 消息类型                                     |
| `sub_type`     | string | `"friend"`、`"group"`、`"discuss"`、`"other"` | 消息子类型，如果是好友则是 `"friend"`，如果从群或讨论组来的临时会话则分别是 `"group"`、`"discuss"` |
| `user_id`      | number | -                                        | 发送者 QQ 号                                 |
| `message`      | string | -                                        | 消息内容                                     |

### 群消息

| 字段名              | 数据类型   | 可能的值        | 说明                        |
| ---------------- | ------ | ----------- | ------------------------- |
| `post_type`      | string | `"message"` | 上报类型                      |
| `message_type`   | string | `"group"`   | 消息类型                      |
| `group_id`       | number | -           | 群号                        |
| `user_id`        | number | -           | 发送者 QQ 号                  |
| `anonymous`      | string | -           | 匿名用户显示名                   |
| `anonymous_flag` | string | -           | 匿名用户 flag，在调用禁言 API 时需要传入 |
| `message`        | string | -           | 消息内容                      |

### 讨论组消息

| 字段名            | 数据类型   | 可能的值        | 说明       |
| -------------- | ------ | ----------- | -------- |
| `post_type`    | string | `"message"` | 上报类型     |
| `message_type` | string | `"discuss"` | 消息类型     |
| `discuss_id`   | number | -           | 讨论组 ID   |
| `user_id`      | number | -           | 发送者 QQ 号 |
| `message`      | string | -           | 消息内容     |

### 群管理员变动

| 字段名         | 数据类型   | 可能的值              | 说明                 |
| ----------- | ------ | ----------------- | ------------------ |
| `post_type` | string | `"event"`         | 上报类型               |
| `event`     | string | `"group_admin"`   | 事件名                |
| `sub_type`  | string | `"set"`、`"unset"` | 事件子类型，分别表示设置和取消管理员 |
| `group_id`  | number | -                 | 群号                 |
| `user_id`   | number | -                 | 发送者 QQ 号           |

### 群成员减少

| 字段名           | 数据类型   | 可能的值                           | 说明                                       |
| ------------- | ------ | ------------------------------ | ---------------------------------------- |
| `post_type`   | string | `"event"`                      | 上报类型                                     |
| `event`       | string | `"group_decrease"`             | 事件名                                      |
| `sub_type`    | string | `"leave"`、`"kick"`、`"kick_me"` | 事件子类型，分别表示主动退群、成员被踢、登录号被踢（但是似乎登录号被踢也会表现为 `"kick"` |
| `group_id`    | number | -                              | 群号                                       |
| `user_id`     | number | -                              | 离开者 QQ 号                                 |
| `operator_id` | number | -                              | 操作者 QQ 号（如果是主动退群，则和 `user_id` 相同）        |

### 群成员增加

| 字段名           | 数据类型   | 可能的值                   | 说明                         |
| ------------- | ------ | ---------------------- | -------------------------- |
| `post_type`   | string | `"event"`              | 上报类型                       |
| `event`       | string | `"group_increase"`     | 事件名                        |
| `sub_type`    | string | `"approve"`、`"invite"` | 事件子类型，分别表示管理员已同意入群、管理员邀请入群 |
| `group_id`    | number | -                      | 群号                         |
| `user_id`     | number | -                      | 加入者 QQ 号                   |
| `operator_id` | number | -                      | 操作者 QQ 号                   |

### 好友已添加

| 字段名         | 数据类型   | 可能的值             | 说明         |
| ----------- | ------ | ---------------- | ---------- |
| `post_type` | string | `"event"`        | 上报类型       |
| `event`     | string | `"friend_added"` | 事件名        |
| `user_id`   | number | -                | 新添加好友 QQ 号 |

### 加好友请求

| 字段名            | 数据类型   | 可能的值        | 说明                         |
| -------------- | ------ | ----------- | -------------------------- |
| `post_type`    | string | `"request"` | 上报类型                       |
| `request_type` | string | `"friend"`  | 请求类型                       |
| `user_id`      | number | -           | 发送请求的 QQ 号                 |
| `message`      | string | -           | 验证信息                       |
| `flag`         | string | -           | 请求 flag，在调用处理请求的 API 时需要传入 |

### 加群请求／邀请

| 字段名            | 数据类型   | 可能的值               | 说明                         |
| -------------- | ------ | ------------------ | -------------------------- |
| `post_type`    | string | `"request"`        | 上报类型                       |
| `request_type` | string | `"group"`          | 请求类型                       |
| `sub_type`     | string | `"add"`、`"invite"` | 请求子类型，分别表示加群请求、邀请登录号入群     |
| `user_id`      | number | -                  | 发送请求的 QQ 号                 |
| `message`      | string | -                  | 验证信息                       |
| `flag`         | string | -                  | 请求 flag，在调用处理请求的 API 时需要传入 |