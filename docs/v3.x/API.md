# API 描述

## 请求方式

所有 API 都同时支持 GET 和 POST 两种请求方式（除获取 `data` 目录中的文件），参数可通过 URL 参数、表单或 JSON 传入，后两者分别对应 Content-Type `application/x-www-form-urlencoded` 和 `application/json`。如果使用 JSON 传入，参数要放在根级别的 JSON 对象中，且参数的数据类型必须符合 API 列表中给出的每个参数所要求的数据类型，大致如下：

```json
{"user_id": 123456, "message": "hello"}
```

**为避免各种可能的问题，建议使用 JSON 方式传入。**

API 描述中没有给出默认值的参数均为必填项。

如果配置文件中填写了 `access_token`，则每次请求需要在请求头中加入验证头，如：

```http
GET /send_private_msg?user_id=123456&message=hello HTTP/1.1
Authorization: Token kSLuTF2GC2Q4q4ugm3
```

`kSLuTF2GC2Q4q4ugm3` 换成你填写的 access token。

Access token 也可以通过 query 参数 `access_token` 传入，以便在无法修改请求头的情况下使用，例如：

```http
GET /send_private_msg?access_token=kSLuTF2GC2Q4q4ugm3&user_id=123456&message=hello HTTP/1.1
```

## 响应说明

对于任何请求:

- 如果 access token 未提供，状态码为 401；
- 如果 access token 不符合，状态码为 403；
- 如果 POST 请求的 Content-Type 不支持，状态码为 406；
- 如果 POST 请求的正文格式不正确，状态码为 400；
- 如果 API 不存在，状态码为 404；
- 剩下的所有情况，无论操作失败还是成功，状态码都是 200。

响应内容为 JSON 格式，基本结构如下：

```json
{
    "status": "ok",
    "retcode": 0,
    "data": {
        "id": 123456,
        "nickname": "滑稽"
    }
}
```

`status` 字段如果是 `ok` 则表示操作成功，同时 `retcode` （返回码）会等于 0，即酷 Q 函数返回了 0，也就是酷 Q 认为成功了，但实际上有某些情况下其实是没有成功的，比如向没有加入的群发送消息，返回码是 0，日志里也没有提示发送失败，但显然是发送不了的。这一点在酷 Q 的 [官方文档](https://d.cqp.me/Pro/%E5%BC%80%E5%8F%91/Error) 也说明了：

>  需要指出的是，某些接口暂未进行错误代码的处理，此时即使发生错误，仍返回0。

`status` 字段如果是 `async` 则表示请求已提交异步处理，此时 `retcode` 为 1，具体成功或失败将无法获知。

`status` 字段如果是 `failed` 则表示操作失败，此时 `retcode` 有两种情况：当大于 0 时，表示是 HTTP API 插件判断出的失败；小于 0 时，为调用酷 Q 函数的返回码，具体含义直接参考 [Pro/开发/Error](https://d.cqp.me/Pro/%E5%BC%80%E5%8F%91/Error) 和酷 Q 的日志。

汇总如下：

| `retcode` | 说明 |
| --------- | ---- |
| 0 | 同时 `status` 为 `ok`，表示操作成功 |
| 1 | 同时 `status` 为 `async`，表示操作已进入异步执行，具体结果未知 |
| 100 | 默认的错误码，通常是因为没有传入必要参数，某些接口中也可能因为参数明显无效（比如传入的 QQ 号小于等于 0，此时无需调用酷 Q 函数即可确定失败），此项和以下的 `status` 均为 `failed` |
| 102 | 酷 Q 函数返回的数据无效，一般是因为传入参数有效但没有权限，比如试图获取没有加入的群组的成员列表 |
| 201 | 工作线程池未正确初始化（无法执行异步任务） |

`data` 字段为 API 返回数据的内容，对于发送消息、踢人等不需要返回数据的操作，这里为 null，对于获取群成员信息这类操作，这里为所获取的数据的对象，具体的数据内容将会在相应的 API 描述中给出。

后面的 API 描述中将只给出 `data` 字段的内容，放在「响应数据」标题下。

## 发送消息格式

从 2.0.0 版本开始发送消息格式有了较大变化（向下兼容），具体请查看 [消息格式](https://richardchien.github.io/coolq-http-api/#/Message)。

后面提到消息的地方（`message` 字段），将不再具体解释其格式。

## API 列表

### `/send_private_msg` 发送私聊消息

有异步版本 `/send_private_msg_async`。

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `user_id` | number | - | 对方 QQ 号 |
| `message` | string/array | - | 要发送的内容 |
| `auto_escape` | bool | false | 消息内容是否作为纯文本发送（即不解析 CQ 码），`message` 数据类型为 `array` 时无效 |

#### 响应数据

无

### `/send_group_msg` 发送群消息

有异步版本 `/send_group_msg_async`。

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `message` | string/array | - | 要发送的内容 |
| `auto_escape` | bool | false | 消息内容是否作为纯文本发送（即不解析 CQ 码），`message` 数据类型为 `array` 时无效 |

#### 响应数据

无

### `/send_discuss_msg` 发送讨论组消息

有异步版本 `/send_discuss_msg_async`。

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `discuss_id` | number | - | 讨论组 ID（正常情况下看不到，需要从讨论组消息上报的数据中获得） |
| `message` | string/array | - | 要发送的内容 |
| `auto_escape` | bool | false | 消息内容是否作为纯文本发送（即不解析 CQ 码），`message` 数据类型为 `array` 时无效 |

#### 响应数据

无

### `/send_like` 发送好友赞

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `user_id` | number | - | 对方 QQ 号 |
| `times` | number | 1 | 赞的次数，每个好友每天最多 10 次 |

#### 响应数据

无

### `/set_group_kick` 群组踢人

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 要踢的 QQ 号  |
| `reject_add_request` | bool | `false` | 拒绝此人的加群请求 |

#### 响应数据

无

### `/set_group_ban` 群组单人禁言

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 要禁言的 QQ 号 |
| `duration` | number | `30 * 60` | 禁言时长，单位秒，0 表示取消禁言 |

#### 响应数据

无

### `/set_group_anonymous_ban` 群组匿名用户禁言

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `flag` | string | - | 要禁言的匿名用户的 flag（需从群消息上报的数据中获得） |
| `duration` | number | `30 * 60` | 禁言时长，单位秒，无法取消匿名用户禁言 |

#### 响应数据

无

### `/set_group_whole_ban` 群组全员禁言

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `enable` | bool | `true` | 是否禁言 |

#### 响应数据

无

### `/set_group_admin` 群组设置管理员

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 要设置管理员的 QQ 号 |
| `enable` | bool | `true` | true 为设置，false 为取消 |

#### 响应数据

无

### `/set_group_anonymous` 群组匿名

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `enable` | bool | `true` | 是否允许匿名聊天 |

#### 响应数据

无

### `/set_group_card` 设置群名片（群备注）

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 要设置的 QQ 号 |
| `card` | string | 空 | 群名片内容，不填或空字符串表示删除群名片 |

#### 响应数据

无

### `/set_group_leave` 退出群组

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `is_dismiss` | bool | `false` | 是否解散，如果登录号是群主，则仅在此项为 true 时能够解散 |

#### 响应数据

无

### `/set_group_special_title` 设置群组专属头衔

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `user_id` | number | - | 要设置的 QQ 号 |
| `special_title` | string | 空 | 专属头衔，不填或空字符串表示删除专属头衔 |
| `duration` | number | `-1` | 专属头衔有效期，单位秒，-1 表示永久，不过此项似乎没有效果，可能是只有某些特殊的时间长度有效，有待测试 |

#### 响应数据

无

### `/set_discuss_leave` 退出讨论组

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `discuss_id` | number | - | 讨论组 ID（正常情况下看不到，需要从讨论组消息上报的数据中获得） |

#### 响应数据

无

### `/set_friend_add_request` 处理加好友请求

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `flag` | string | - | 加好友请求的 flag（需从上报的数据中获得） |
| `approve` | bool | `true` | 是否同意请求 |
| `remark` | string | 空 | 添加后的好友备注（仅在同意时有效） |

#### 响应数据

无

### `/set_group_add_request` 处理加群请求／邀请

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `flag` | string | - | 加好友请求的 flag（需从上报的数据中获得） |
| `type` | string | - | `add` 或 `invite`，请求类型（需要和上报消息中的 `sub_type` 字段相符） |
| `approve` | bool | `true` | 是否同意请求／邀请 |
| `reason` | string | 空 | 拒绝理由（仅在拒绝时有效） |

#### 响应数据

无

### `/get_login_info` 获取登录号信息

#### 参数

无

#### 响应数据

| 字段名 | 数据类型 | 说明 |
| ----- | ------- | --- |
| `user_id` | number | QQ 号 |
| `nickname` | string | QQ 昵称 |

### `/get_stranger_info` 获取陌生人信息

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `user_id` | number | - | QQ 号（不可以是登录号） |
| `no_cache` | bool | `false` | 是否不使用缓存（使用缓存可能更新不及时，但响应更快） |

#### 响应数据

| 字段名 | 数据类型 | 说明 |
| ----- | ------- | --- |
| `user_id` | number | QQ 号 |
| `nickname` | string | 昵称 |
| `sex` | string | 性别，`male` 或 `female` 或 `unknown` |
| `age` | number | 年龄 |

### `/get_group_list` 获取群列表

#### 参数

无

#### 响应数据

响应内容为 JSON 数组，每个元素如下：

| 字段名 | 数据类型 | 说明 |
| ----- | ------- | --- |
| `group_id` | number | 群号 |
| `group_name` | string | 群名称 |

### `/get_group_member_info` 获取群成员信息

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |
| `user_id`  | number | - | QQ 号（不可以是登录号） |
| `no_cache` | bool | `false` | 是否不使用缓存（使用缓存可能更新不及时，但响应更快） |

#### 响应数据

| 字段名 | 数据类型 | 说明 |
| ----- | ------- | --- |
| `group_id` | number | 群号 |
| `user_id` | number | QQ 号 |
| `nickname` | string | 昵称 |
| `card` | string | 群名片／备注 |
| `sex` | string | 性别，`male` 或 `female` 或 `unknown` |
| `age` | number | 年龄 |
| `area` | string | 地区 |
| `join_time` | number | 加群时间戳 |
| `last_sent_time` | number | 最后发言时间戳 |
| `level` | string | 成员等级 |
| `role` | string | 角色，`owner` 或 `admin` 或 `member` |
| `unfriendly` | bool | 是否不良记录成员 |
| `title` | string | 专属头衔 |
| `title_expire_time` | number | 专属头衔过期时间戳 |
| `card_changeable` | bool | 是否允许修改群名片 |

### `/get_group_member_list` 获取群成员列表

#### 参数

| 字段名 | 数据类型 | 默认值 | 说明 |
| ----- | ------- | ----- | --- |
| `group_id` | number | - | 群号 |

#### 响应数据

响应内容为 JSON 数组，每个元素的内容和上面的 `/get_group_member_info` 接口相同，但对于同一个群组的同一个成员，获取列表时和获取单独的成员信息时，某些字段可能有所不同，例如 `area`、`title` 等字段在获取列表时无法获得，具体应以单独的成员信息为准。

### `/get_cookies` 获取 Cookies

#### 参数

无

#### 响应数据

| 字段名 | 数据类型 | 说明 |
| ----- | ------- | --- |
| `cookies` | string | Cookies |

### `/get_csrf_token` 获取 CSRF Token

#### 参数

无

#### 响应数据

| 字段名 | 数据类型 | 说明 |
| ----- | ------- | --- |
| `token` | number | CSRF Token |

### `/get_version_info` 获取酷 Q 及 HTTP API 插件的版本信息

#### 参数

无

#### 响应数据

| 字段名 | 数据类型 | 说明 |
| ----- | ------- | --- |
| `coolq_directory` | string | 酷 Q 根目录路径 |
| `coolq_edition` | string | 酷 Q 版本，`air` 或 `pro` |
| `plugin_version` | string | HTTP API 插件版本，例如 `2.1.3` |
| `plugin_build_number` | number | HTTP API 插件 build 号 |
| `plugin_build_configuration` | string | HTTP API 插件编译配置，`debug` 或 `release` |

## 获取 `data` 目录中的文件的接口

2.1.0 版本开始提供一个简单的静态文件获取服务，请求方式只支持 GET，URL 路径为 `/data/` 加上要请求的文件相对于酷 Q `data` 目录的路径。例如，假设酷 Q 主目录在 `C:\Apps\CQA`，则要获取 `C:\Apps\CQA\data\image\ABCD.jpg.cqimg` 的话，只需请求 `/data/image/ABCD.jpg.cqimg`，响应内容即为要请求的文件。

和上面的其它请求一样，如果配置文件中指定了 access token，则每次请求需要在请求头中加入验证头 `Authorization: Token your-token`。

另外，请求的路径中不允许出现 `..`，即上级目录的标记，以防止恶意或错误的请求到系统中的其它文件。

本功能默认情况下不开启，在配置文件中将 `serve_data_files` 设置为 `yes` 或 `true` 即可开启，见 [配置文件说明](https://richardchien.github.io/coolq-http-api/#/Configuration)。
