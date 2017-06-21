# HTTPS

部分用户在公网环境下使用 HTTP API 插件的用户可能会对 HTTPS 有需求，但目前 HTTP API 插件本身不支持直接配置 SSL/TLS 证书，在可预见的未来也不会加入这个功能。因此，要使用 HTTPS 需要使用第三方软件来进行代理，除了广为人知的 Nginx、Apache 等，推荐使用 [Caddy](https://caddyserver.com/) 这个轻量且易用的 web 服务器软件来支持 HTTPS。

推荐 Caddy 的原因主要有下面几个：

- 主程序为单个可执行文件，且体积不大
- 支持多种平台，Linux、Windows、macOS 等
- 可以自动从 Let's Encrypt 签发证书
- 配置文件编写非常简单

## 配置方式（使用 Caddy 为例）

我们现在假设你有一个酷 Q 运行在一台 Windows Server 上，监听 `127.0.0.1:5700`，解析到服务器公网 IP 的一个域名是 `example.com`，现在你想在另一台非内网服务器上通过 HTTPS 协议访问 `example.com:5701` 以调用 HTTP API 插件的 [API 接口](https://richardchien.github.io/coolq-http-api/#/API)。

通过 Caddy，你只需要两行或三行配置即可实现上述需求。

首先前往 https://caddyserver.com/download 下载你需要的平台的 Caddy 程序（不需要添加 plugin），将解压之后的 `caddy.exe` 或其它平台的可执行文件放到环境变量 PATH 的某个路径中（也可以不用，而直接使用绝对或相对路径运行）。

在系统的任意位置（建议在酷 Q 的 `app\io.github.richardchien.coolqhttpapi` 中）创建文件 `caddyfile`，并加入下面配置内容：

```
https://example.com:5701
proxy / 127.0.0.1:5700
```

然后切换目录到这个文件所在的目录，运行：

```sh
$ caddy
```

第一次运行会提示输入邮箱，然后进行 ACME Challenge，成功后签发了 SSL 证书，此后运行将直接出现：

```sh
$ caddy
Activating privacy features... done.
https://example.com:5701
http://example.com
```

至此 HTTPS 已经配置完毕。

如果已有 SSL 证书，不需要自动签发证书的话，可以在配置文件中加上下面一行即可：

```
tls cert.pem key.pem
```

另外，在公网上使用 HTTP API 插件，请一定要配置一个较复杂的 token，以防止遭到恶意攻击。
