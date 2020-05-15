importScripts(
  'https://storage.googleapis.com/workbox-cdn/releases/3.6.1/workbox-sw.js'
)

const matchCb = ({ url, event }) => {
  return true
}

workbox.routing.registerRoute(
  matchCb,
  workbox.strategies.networkFirst()
)
