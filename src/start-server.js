import { Server } from './socket/server.js';
import { log, inspect } from './util/debug.js';

const server = new Server();

server.on('connect', (socket) => {
  log(`connected to ${socket}`);
});
server.on('message', (socket, message) => {
  log(`message received from ${socket}: %s`, message);
});
server.on('close', (why) => {
  log(`socket closed: %s`, why);
});
server.on('pong', (socket, data) => {
  log(`pong received from ${socket}, %s`, data);
});

server.listen(7981);
log(`test`);