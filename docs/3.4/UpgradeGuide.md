# 升级指南

由于 3.x 版本引入了一些 breaking change，因此这里给出一份升级指南，帮助用户从旧版升级到 3.x。

## 移除 `pattern`、`post_timeout` 配置项

如果你在旧版配置了这两个配置项，则在 3.0 版开始将不再有效。

## `is_raw` 参数更名为 `auto_escape`

如果你的代码在调用旧版 API 时使用了 `is_raw` 参数来让插件自动转义消息内容，则只需将参数名修改为 `auto_escape`。

## `token` 配置项更名为 `access_token`

如果你在旧版配置了 `token` 配置项，则现在需要改为 `access_token`。且此配置项现只用于 API 调用的鉴权（同旧版），见下条。

## Access Token 不再用于上报消息

旧版中上报消息时插件会在请求头中加入形如 `Authorization: token kSLuTF2GC2Q4q4ugm3` 的验证头，用于验证上报消息确实来自于插件，如果你使用了这个来验证，则在 3.0 版本开始将不再可用。

3.0 版本改用 HMAC SHA1 签名的方式实现同样的验证功能，你需要在配置项中填写 `secret` 项，设置一个密钥，然后插件在上报时会用这个密钥，使用 HMAC SHA1 算法来对 HTTP 正文（即要上报的 JSON 数据）进行哈希，得出的哈希值被放在 `X-Signature` 请求头中，例如：

```http
POST / HTTP/1.1
X-Signature: sha1=f9ddd4863ace61e64f462d41ca311e3d2c1176e2
```

`sha1=` 后面即为哈希值。通过使用同一密钥对收到的的 HTTP 请求正文进行哈希即可知道该请求是否确实来自插件。

例如在 Python 中可以这样判断：

```python
import hmac
from flask import Flask, request

app = Flask(__name__)

@app.route('/', methods=['POST'])
def receive():
    sig = hmac.new(b'<your-key>', request.get_data(), 'sha1').hexdigest()
    received_sig = request.headers['X-Signature'][len('sha1='):]
    if sig == received_sig:
        # 请求确实来自于插件
        pass
    else:
        # 假的上报
        pass
```

## Access Token 不符分为 401 和 403

如果你的代码原先对调用 API 时 access token 不符的 HTTP 状态码进行了判断，则现在对于没有提供 token 的情况，返回 401，提供了但是不相符的情况，返回 403。

## Docker 镜像的变化

Docker 镜像不再使用 `CQHTTP_VERSION` 环境变量来指定插件版本，而是直接使用镜像的 tag 来指定，例如，如果你要使用 3.0.0 版本的插件，则直接拉取 `richardchien/cqhttp:3.0.0` 镜像。并且，默认情况下，手动更换或升级 cpk 文件、修改配置文件会受到限制，见插件文档的 [Docker](/Docker) 部分。
