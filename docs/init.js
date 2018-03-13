var rootUrl = 'https://cqhttp.cc/docs';

var versions = [
    { title: 'v3.4 (latest)', path: '/3.4/' },
    { title: 'v3.3', path: '/3.3/' },
    { title: 'v3.2', path: '/3.2/' },
    { title: 'v3.1', path: '/3.1/' },
    { title: 'v3.0', path: '/3.0/' },
    { title: 'v1.x-v2.x', path: '/legacy/' }
]

var latestVersionIndex = 0;
var currentVersionIndex = -1;

versions.forEach(function (v, idx) {
    if (v.path) {
        var pathname = window.location.pathname;
        if (pathname.lastIndexOf(v.path) == pathname.length - v.path.length /* pathname.endsWith(v.path) */) {
            currentVersionIndex = idx;
            rootUrl = window.location.origin + pathname.substr(0, pathname.length - v.path.length);
        }
    }
});

versions.forEach(function (v) {
    if (v.path) {
        v.path = rootUrl + v.path;
    }
});

config.nav.push({
    title: '文档版本: ' + versions[currentVersionIndex].title, type: 'dropdown', items: versions
});

if (currentVersionIndex > latestVersionIndex) {
    config.announcement = {
        type: 'danger',
        html: '你当前正在访问的是旧版本文档，内容可能与最新版本的插件不相符，点击&nbsp;<span style="cursor: pointer;" '
            + 'onclick="window.location.assign(\'' + versions[latestVersionIndex].path + '\' + window.location.hash)"'
            + '>这里</span>&nbsp;访问最新文档。'
    };
}

docute.init(config);

// Google Analytics
window.ga = window.ga || function () { (ga.q = ga.q || []).push(arguments) }; ga.l = +new Date;
ga('create', 'UA-115509121-1', 'auto');
ga('send', 'pageview');
docute.router.afterEach(function (to) {
    ga('set', 'page', to.fullPath);
    ga('send', 'pageview');
});
