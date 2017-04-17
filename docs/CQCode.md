# CQ 码

CQ 码的使用方式和酷 Q 原生相同，在要发送的消息中插入相应的代码即可，例如：

```
[CQ:face,id=14] [CQ:image,file=1.jpg]
```

使用方式和原生 CQ 码（关于原生 CQ 码的使用，请看 [Pro/CQ码](https://d.cqp.me/Pro/CQ%E7%A0%81)）完全相同**意味着需要在某些情况下对字符进行转义**，由于很多时候我们不需要使用 CQ 码，只需要发送文字消息就行了，这种情况下可以在请求 API 时加入 `is_raw` 参数，这将会自动对整个消息的特殊字符进行转义，具体请看 [API 描述](https://richardchien.github.io/coolq-http-api/#/API)。

除了原生的 CQ 码，CoolQ HTTP API 还提供了一些实用的增强功能，后面称之为「增强 CQ 码」。

## 增强功能列表

### 发送网络图片或语音

酷 Q 原生的 CQ 码只能发送 `data\image` 文件夹里的图片、`data\record` 里的语音，增强 CQ 码支持设置 `file` 为网络链接，内部会首先把图片或语音下载到 `data` 中相应的文件夹，然后把 `file` 替换成下载好的本地文件名。例如：

```
[CQ:image,file=http://i1.piimg.com/567571/fdd6e7b6d93f1ef0.jpg]
[CQ:record,file=http://doora.qiniudn.com/35aIm.silk]
```

### 发送文件系统中另一个地方的图片或语音

除了发送网络上的图片、语音，还可以发送本地文件系统中其它地方的图片、语音，使用 `file://` 加文件的绝对路径，例如：

```
[CQ:image,file=file://C:\Users\richard\Pictures\1.png]
[CQ:record,file=file://C:\Users\richard\Music\1.mp3]
```

### 提取 cqimg 文件中的实际图片 URL

酷 Q 收到的图片会放在 `data\image` 中，并且以文件名加 `.cqimg` 扩展名的形式存储为纯文本文件，实际的图片 URL 就在里面的 `url` 字段，增强 CQ 码会自动提取这个 URL，并添加到 CQ 码的 `url` 参数中。

例如，假设原 CQ 码如下：

```
[CQ:image,file=AE3062186A2073B33AB1F2BB2F58F3A4.jpg]
```

提取 URL 后，会更改为：

```
[CQ:image,file=AE3062186A2073B33AB1F2BB2F58F3A4.jpg,url=http://183.232.95.26/offpic_new/1002647525//8102132e-4ab0-46cf-a8e1-2f62185232cb/0]
```

如果提取不成功（读取文件失败），则不变。

**另外请注意，这个 URL 会在一定时间后过期（不确定多久），但酷 Q 在收到以前收过的图之后，仍然会返回同样的文件名，因此建议定期清理 `data\image` 目录以防止 URL 失效。**
