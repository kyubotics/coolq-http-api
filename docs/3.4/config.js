docute.init({
    title: 'CoolQ HTTP API 插件文档',
    home: 'Home.md',
    repo: 'richardchien/coolq-http-api',
    nav: [
        {
            title: '首页', path: '/'
        },
        {
            title: 'Docker', path: '/Docker'
        },
        {
            title: '配置', path: '/Configuration'
        },
        {
            title: '通信方式', path: '/CommunicationMethods'
        },
        {
            title: 'API', type: 'dropdown', items: [
                {
                    title: 'API 描述', path: '/API'
                },
                {
                    title: 'WebSocket API 描述', path: '/WebSocketAPI'
                }
            ]
        },
        {
            title: '事件', type: 'dropdown', items: [
                {
                    title: '事件上报', path: '/Post'
                },
                {
                    title: '事件过滤器', path: '/EventFilter'
                }
            ]
        },
        {
            title: '消息内容', type: 'dropdown', items: [
                {
                    title: '消息格式', path: '/Message'
                },
                {
                    title: 'CQ 码', path: '/CQCode'
                }
            ]
        },
        {
            title: 'HTTPS', path: 'https://github.com/richardchien/coolq-http-api/wiki/HTTPS'
        },
        {
            title: 'FAQ', path: 'https://github.com/richardchien/coolq-http-api/wiki/FAQ'
        }
    ],
    tocVisibleDepth: 2,
    plugins: []
});
