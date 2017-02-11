# CQ 码

CQ 码的使用方式和酷 Q 原生相同，在要发送的消息中插入相应的代码即可，例如：

```
[CQ:face,id=14] [CQ:image,file=1.jpg]
```

使用方式和原生 CQ 码（关于原生 CQ 码的使用，请看 [Pro/CQ码](https://d.cqp.me/Pro/CQ%E7%A0%81)）完全相同意味着需要在某些情况下对字符进行转义，由于很多时候我们不需要使用 CQ 码，只需要发送文字消息就行了，这种情况下可以在请求 API 时加入 `is_raw` 参数，这将会自动对整个消息的特殊字符进行转义，具体请看 [API 描述](https://richardchien.github.io/coolq-http-api/#/API)。

除了原生的 CQ 码，CoolQ HTTP API 还提供了一些实用的增强功能，后面称之为「增强 CQ 码」。

## 增强功能列表

目前只有一个增强功能。

### 发送网络图片

酷 Q 原生的 CQ 码只能发送 `data\image` 文件夹里的图片，增强 CQ 码支持设置 `file` 为图片链接，内部会首先把图片下载到 `data\image` 文件夹，然后把 `file` 替换成下载好的本地文件。例如：

```
[CQ:image,file=http://i1.piimg.com/567571/fdd6e7b6d93f1ef0.jpg]
```
