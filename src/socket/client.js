import { inspect, log } from '../util/debug.js';
import { client } from '/usr/local/lib/libminnet.so';
import * as std from 'std';

export class Client {
  constructor(port, host, onDemand) {
    this.host = host;
    this.port = port;
    this.onDemand = onDemand;
    if (!onDemand) {
      this.client = this.createClient();
      log(`Client created: %s`, inspect(this.client));
    } else {
      log(`Ready to connect to ${host}:${port} on demand`);
    }
  }
  createClient() {
    return client({
      port: this.port,
      host: this.host,
      onConnect: (socket) => {
        log(`Connected to server %s: %s`, this.host, inspect(socket));
      },
      onMessage: (socket, msg) => {
        log(`Received from server: %s`, inspect(msg));
      },
      onClose: (why) => {
        log(`The server disconnected. Reason: %s`, why);
      },
      onPong: (socket, data) => {
        log(`Pong: %s`, inspect(data));
      }
    });
  }
  send(message) {
    if (this.onDemand) {
      this.client = this.createClient();
    }
    log(`Sending a message %s`, message);
    this.client.socket.send(message);
    if (this.onDemand) {
      this.close('onDemand socket');
    }
  }
  close(reason) {
    this.client.socket.close(reason);
    this.client = null;
  }
  pong(data) {
    if (this.onDemand) {
      this.client = this.createClient();
    }
    this.client.socket.pong(data);
    if (this.onDemand) {
      this.close('onDemand socket');
    }
  }
}
