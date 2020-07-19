### 2020-07-19
- The server now raises 'listening' event and callback when it is listening
- Added a debug logger.  The container sets **ENV QJSNET_DEBUG=true** to enable it automatically.  Reduce the chatter with ```unset QJSNET_DEBUG``` 