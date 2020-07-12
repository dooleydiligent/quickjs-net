import { bind, socket } from '/usr/local/lib/libqjsnet.so';

const socket1 = socket();
console.log(`Socket1 is ${socket1}`);

const socket2 = socket();
console.log(`Socket2 is ${socket2}`);

// bind to socket1 on port 3000
if (bind({ socket: socket1, port: 3000, host: "localhost"}) !== socket1) {
  console.log(`Error binding to socket ${socket1} on port 3000`);
} else {
  console.log(`Bind to port 3000 successful!`);
}

// fail to bind to socket2 on port 3000
if (bind({ socket: socket2, port: 3000, host: "localhost"}) !== socket2) {
  console.log(`Error binding to socket ${socket2} on port 3000`);
} else {
  console.log(`Bind to port 3000 successful for socet2.  This is an error!`);
}
