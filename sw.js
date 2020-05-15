importScripts(
  'https://storage.googleapis.com/workbox-cdn/releases/3.6.1/workbox-sw.js'
)

const ALLOWED_HOSTS = [
  'cqhttp.cc',
  'cqhttp.bleatingsheep.org',
  'img.shields.io',
  'raw.githubusercontent.com',
  'unpkg.com'
]

const matchCb = ({ url, event }) => {
  return true
}

workbox.routing.registerRoute(
  matchCb,
  workbox.strategies.networkFirst()
)
