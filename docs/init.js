let rootUrl = 'https://cqhttp.cc/docs';

let versions = [
    { title: 'v3.4 (latest)', path: '/3.4/' },
    { title: 'v3.3', path: '/3.3/' },
    { title: 'v3.2', path: '/3.2/' },
    { title: 'v3.1', path: '/3.1/' },
    { title: 'v3.0', path: '/3.0/' },
    { title: 'v1.x-v2.x', path: '/legacy/' }
]

let latestVersionIndex = 0;
let currentVersionIndex = -1;

for (const [idx, v] of versions.entries()) {
    if (v.path) {
        const pathname = window.location.pathname;
        if (pathname.endsWith(v.path)) {
            currentVersionIndex = idx;
            rootUrl = window.location.origin + pathname.substr(0, pathname.length - v.path.length);
        }
    }
}

for (const v of versions) {
    if (v.path) {
        v.path = rootUrl + v.path;
    }
}

config.nav.push({
    title: `文档版本: ${versions[currentVersionIndex].title}`, type: 'dropdown', items: versions
});

if (currentVersionIndex > latestVersionIndex) {
    config.announcement = {
        type: 'danger',
        html: `你当前正在访问的是旧版本文档，内容可能与最新版本的插件不相符，点击&nbsp;<a href="${versions[latestVersionIndex].path + window.location.hash}">这里</a>&nbsp;访问最新文档。`
    };
}

docute.init(config);
