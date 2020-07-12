import { inspect, log } from '../util/debug.js';
import { server } from '/usr/local/lib/libminnet.so';
import * as std from 'std';
import { EventEmitter } from '../util/event-emitter.js';
import { Dictionary } from '../util/dictionary.js';

export class Server extends EventEmitter {
  constructor() {
    super();
    this.settings = new Dictionary();
    const env = std.getenv('QJS_ENV') || 'development';
    this.settings.put('env', env);
  }
  set(key, value) {
    if (value === undefined) {
      return this.settings.get(key);
    }
    log('set "%s" to %o', key, value);
    this.settings.put(key, value);
    return this;
  }
  listen(port) {
    log(`Server listening on port %d`, port);

    return server({
      port,
      onConnect: (socket) => {
        log('Client connected %s', socket);
        this.emit('connect', socket);
      },
      onMessage: (socket, msg) => {
        log('Received: %s', msg);
        this.emit('message', socket, msg);
      },
      onClose: (why) => {
        log('Client disconnected. Reason: "%s"', inspect(why));
        this.emit('close', why);
      },
      onPong: (socket, data) => {
        log('Pong: %s', inspect(data));
        this.emit('pong', socket, data);
      }
    });
  }
}
