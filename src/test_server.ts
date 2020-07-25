import { AF_INET, AF_INET6, Server } from '/usr/local/lib/libqjsnet.so';
import * as os from 'os';
import * as std from 'std';

console.log(`AF_INET is ${AF_INET}`);
console.log(`AF_INET6 is ${AF_INET6}`);

const workerarray = [];

const assert = (b, str) => {
  if (b) {
    return;
  } else {
    throw Error("assertion failed: " + str);
  }
}

const startworkers = async (svr) => {
  try {
    console.log(`Starting ${svr.max_connections} worker threads`);
    for (let workers = 0; workers < svr.max_connections; workers++) {
      const w = await new os.Worker(`
      import * as os from 'os';
      import * as std from 'std';

      const parent = os.Worker.parent;
      let socketId = 0;
      const handle_msg = (e) => {
        const ev = e.data;
        print("child_recv", JSON.stringify(ev));
        switch(ev.type) {
          case "socket":
            socketId = ev.socket;
            console.log("SocketId is " + socketId);
            break;
          case "abort":
            parent.postMessage({ type: "done" });
            break;
          case "sab":
            /* modify the SharedArrayBuffer */
            ev.buf[2] = 10;
            parent.postMessage({ type: "sab_done", buf: ev.buf });
            break;
        }
      };

      const worker_main = () => {
        console.log("worker_main()");
        parent.onmessage = handle_msg;
        parent.postMessage({ type: "socket" });
        // for(let i = 1; i < 11; i++) {
        //     parent.postMessage({ type: "num", num: i }); 
        // }
      };
      
      try {
        console.log("Inside worker");
        worker_main();
      } catch (ex) {
        console.log("Worker exception: ", ex);
      }
`);
      let counter = 0;
      const socketId = svr.socket;
      const port = svr.port;
      const address = svr.address;
      console.log(`On worker start: address:port:socket = ${address}:${port}:${socketId}`);
      w.onmessage = (e) => {
        const ev = e.data;
        console.log("recv", JSON.stringify(ev));
        switch (ev.type) {
          case "socket":
            // The worker is requesting the socket id
            w.postMessage({ type: "socket", socket: socketId });
            break;
          case "num":
            assert(ev.num, counter);
            counter++;
            if (counter == 10) {
              /* test SharedArrayBuffer modification */
              let sab = new SharedArrayBuffer(10);
              let buf = new Uint8Array(sab);
              w.postMessage({ type: "sab", buf: buf });
            }
            break;
          case "sab_done":
            {
              let buf = ev.buf;
              /* check that the SharedArrayBuffer was modified */
              assert(buf[2], 10);
              w.postMessage({ type: "abort" });
            }
            break;
          case "done":
            /* terminate */
            w.onmessage = null;
            break;
        }
      };

      workerarray.push(w);
      console.log(`There are ${workerarray.length} workers`);
    }
  } catch (ex) {
    console.log(`Unexpected exception ${ex}`);
  }
}
const main = async () => {
  // console.log(`Validate the server can be instantiated with no parameters (e.g. new Server())`);
  // let svr = new Server();
  // console.log(`server instantiated`);
  // assert(svr.port === 7981, `Expected port to be 7981 but found ${svr.port}`);
  // console.log(`Found port ${svr.port}`);
  // assert(svr.address === "127.0.0.1", `Expected address to be '127.0.0.1' but found ${svr.address}`);
  // assert(svr.max_connections === 5, `Expected max_connections to be 5 but found ${svr.max_connections}`);
  // svr.close();

  // console.log(`Validate the server can be instantiated with only a port (e.g. new Server(3000))`);
  // svr = new Server(3000);
  // console.log(`Checking port`);
  // console.log(`Port is ${svr.port}`);
  // assert(svr.port === 3000, `Expected port to be 3000 but found ${svr.port}`);
  // console.log(`Checking address`);
  // assert(svr.address === "127.0.0.1", `Expected address to be '127.0.0.1' but found ${svr.address}`);
  // console.log(`closing`);
  // svr.close();
  // console.log(`closed`);

  // console.log(`Validate the server can be instantiated with a port and an host (e.g. new Server(4000, "localhost"))`);
  // svr = new Server(4000, "localHost");
  // assert(svr.port === 4000, `Expected port to be 4000 but found ${svr.port}`);
  // assert(svr.address === "127.0.0.1", `Expected address to be '127.0.0.1' but found ${svr.address}`);
  // svr.close();

  // console.log(`Validate the server can be instantiated with a port and an address (e.g. new Server(5000, '0.0.0.0'))`);
  // svr = new Server(5000, "0.0.0.0");
  // assert(svr.port === 5000, `Expected port to be 5000 but found ${svr.port}`);
  // let address = svr.address;
  // console.log(`Address is ${address}`);
  // assert(svr.address.length === 7, `Expected svr.address.length to be 7 but found ${svr.address.length}`);
  // for (let i = 0; i < address.length; i += 2) {
  //   assert(address[i] === '0', `Expected '0' but found "${address[i]}"`);
  //   if (i + 1 < address.length) {
  //     assert(address[i + 1] === '.', `Expected '.' but found "${address[i + 1]}"`);
  //   }
  // }
  // // TODO: Understand why this does not work.  Probably over doing it with AF_INET and useip
  // assert(svr.address == '0.0.0.0', `Expected address to be '0.0.0.0' but found [${svr.address}]`);
  // svr.close();

  // console.log(`Validate the server can be instantiated with a port and an address (e.g. new Server(5000, '192.168.1.2'))`);
  // svr = new Server(5000, "0.0.0.0");
  // assert(svr.port === 5000, `Expected port to be 5000 but found ${svr.port}`);
  // address = svr.address;
  // console.log(`Address is ${address}`);
  // assert(svr.address.length === 7, `Expected svr.address.length to be 7 but found ${svr.address.length}`);
  // for (let i = 0; i < address.length; i += 2) {
  //   assert(address[i] === '0', `Expected '0' but found "${address[i]}"`);
  //   if (i + 1 < address.length) {
  //     assert(address[i + 1] === '.', `Expected '.' but found "${address[i + 1]}"`);
  //   }
  // }
  // // TODO: Understand why this does not work.  Probably over doing it with AF_INET and useip
  // assert(svr.address == '0.0.0.0', `Expected address to be '0.0.0.0' but found [${svr.address}]`);
  // svr.close();

  // console.log(`Validate the server cannot be instantiated with a port and an invalid address (e.g. new Server(6000, '999.999.999.999'))`);
  // try {
  //   svr = new Server(6000, "999.999.999.999");
  //   assert(svr == 1, "This cannot happen");
  // } catch (ex) {
  //   assert(ex == "TypeError: Exception instantiating Server: Invalid host address", `Unexpected exception: ${ex}`);
  // }
  let message = "The listening callback was not invoked";
  // Listen tests
  console.log(`Validate the server can start workers`);
  const svr2 = await new Server();

  svr2.on('listening', async () => {
      try {

        message = `The listening callback was invoked`;
        console.log(`this.constructor.name = ${svr2.constructor.name}`);
        svr2.max_connections = 1;


        await startworkers(svr2);
        os.setTimeout(() => {
          console.log(`This should cause the application to exit gracefully`);
          workerarray[0].postMessage({ type: "abort" });
        }, 5000);
      } catch (ex) {
        console.log(`Caught exception in listening: ${ex}`);
      }
    })
    .listen();
  // console.log(`Returned from listen`);
  // assert(svr2.address == '127.0.0.1', `Expected address to be '127.0.0.1' but found [${svr2.address}]`);
  // assert(message === "The listening callback was invoked", message);
  // svr2.close();
  // console.log(`If you can see this then it passed`);

}
console.log(`Beginning`);
main();
console.log(`Done`);
