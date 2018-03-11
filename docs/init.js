let rootUrl = 'https://cqhttp.cc/docs';

let versions = [
    { title: 'v3.4', path: '/3.4/' },
    { title: 'v3.3', path: '/3.3/' },
    { title: 'v3.2', path: '/3.2/' },
    { title: 'v3.1', path: '/3.1/' },
    { title: 'v3.0', path: '/3.0/' },
    { title: 'v1.x-v2.x', path: '/legacy/' }
]

let currentVersion = '无';

for (let v of versions) {
    if (v.path) {
        const pathname = window.location.pathname;
        if (pathname.endsWith(v.path)) {
            currentVersion = v.title;
            rootUrl = window.location.origin + pathname.substr(0, pathname.length - v.path.length);
        }
    }
}

for (let v of versions) {
    if (v.path) {
        v.path = rootUrl + v.path;
    }
}

config.nav.push({
    title: `文档版本: ${currentVersion}`, type: 'dropdown', items: versions
});

docute.init(config);
