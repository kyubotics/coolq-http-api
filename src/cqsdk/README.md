# CoolQ C++ SDK

CoolQ C++ SDK 封装了跟 DLL 接口相关的底层逻辑、提供了更现代的 C++ 接口从而为更方便地编写插件提供可能。

具体文档暂时就不写了，顺着 [`cqsdk.h`](cqsdk.h) 头文件找进去基本就可以看明白代码。

请注意你再编写自己的插件时请确保你的 JSON 描述文件和 [`io.github.richardchien.coolqhttpapi.json`](../../io.github.richardchien.coolqhttpapi.json) 文件的 `event` 中的 `function` 字段完全一致，因为 DLL 导出函数名已经在 [`app.cpp`](app.cpp) 和 [`event.cpp`](event.cpp) 写死了。

## 示例

```cpp
#include "cqsdk/cqsdk.h"

namespace app = cq::app;
namespace event = cq::event;
namespace api = cq::api;
namespace logging = cq::logging;
namespace message = cq::message;

// 初始化 App ID
CQ_INITIALIZE("io.github.richardchien.coolqhttpapi");

// 插件入口，在静态成员初始化之后，app::on_initialize 事件发生之前被执行，用于配置 SDK 和注册事件回调
CQ_MAIN {
    cq::config.convert_unicode_emoji = true; // 配置 SDK 自动转换 Emoji 到 Unicode（默认就是 true）

    app::on_enable = []() {
        // logging、api、dir 等命名空间下的函数只能在事件回调函数内部调用，而不能直接在 CQ_MAIN 中调用
        logging::debug(u8"启用", u8"插件已启动");
    };

    event::on_private_msg = [](const cq::PrivateMessageEvent &e) {
        logging::debug(u8"消息", u8"收到私聊消息：" + e.message + u8"，发送者：" + std::to_string(e.user_id));

        api::send_private_msg(e.user_id, e.message); // echo 回去
        api::send_msg(e.target, e.message); // 使用 e.target 指定发送目标

        // MessageSegment 类提供一些静态成员函数以快速构造消息段
        cq::Message msg = cq::MessageSegment::contact(cq::MessageSegment::ContactType::GROUP, 201865589);
        msg.send(e.target); // 使用 Message 类的 send 成员函数

        e.block(); // 阻止事件继续传递给其它插件
    };

    event::on_group_msg = [](const auto &e /* 使用 C++ 的 auto 关键字 */) {
        const auto memlist = api::get_group_member_list(e.group_id); // 获取数据接口
        cq::Message msg = u8"本群一共有 "; // string 到 Message 自动转换
        msg += std::to_string(memlist.size()) + u8" 个成员"; // Message 类可以进行加法运算
        message::send(e.target, msg); // 使用 message 命名空间的 send 函数
    };
}

// 添加菜单项，需要同时在 <appid>.json 文件的 menu 字段添加相应的条目，function 字段为 menu_demo_1
CQ_MENU(menu_demo_1) {
    api::send_private_msg(10000, "hello");
}

// 不像 CQ_INITIALIZE 和 CQ_MAIN，CQ_MENU 可以多次调用来添加多个菜单
CQ_MENU(menu_demo_2) {
    logging::info(u8"菜单", u8"点击了示例菜单2");
}
```

## 生命周期

```
+-----------------------------------------+
|             Enabled At Start            |
+-----------------------------------------+
| on_initialize                           |
|       +                                 |
|       |                                 |
|       v                                 |
|   on_start                              |
|       +                                 |
|       |                                 |
|       v     disabled by user            |
|   on_enable +--------------> on_disable |
|       +                           +     |
|       |                           |     |
|       v                           |     |
|    on_exit <----------------------+     |
+-----------------------------------------+

+---------------------------------------+
|            Disabled At Start          |
+---------------------------------------+
| on_initialize +------+                |
|       +              |enabled by user |
|       |              |                |
|       |              v                |
|       |          on_start             |
|       |              +                |
|       |              |                |
|       |              v                |
|       |          on_enable            |
|       |              +                |
|       |              |                |
|       v              |                |
|    on_exit <---------+                |
+---------------------------------------+
```
