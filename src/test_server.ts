import { AF_INET, AF_INET6, Server } from "/usr/local/lib/libqjsnet.so";
import * as os from "os";
import * as std from "std";

console.log(`AF_INET is ${AF_INET}`);
console.log(`AF_INET6 is ${AF_INET6}`);

const workerarray = [];

const assert = (b, str) => {
  if (b) {
    return;
  } else {
    throw Error("assertion failed: " + str);
  }
};

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
          case "shutdown":
            console.log("worker posting 'shutdown' to parent");
            parent.postMessage({ type: "done" });
            console.log("Worker posted 'shutdown' message");
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
      console.log(
        `On worker start: address:port:socket = ${address}:${port}:${socketId}`
      );
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
              w.postMessage({ type: "shutdown" });
              console.log("Parent posted shutdown");
            }
            break;
          case "done":
            /* terminate */
            console.log(`Shutting down`);
            w.onmessage = null;
            console.log(`Exiting now`);
            break;
        }
      };
      workerarray.push(w);
      console.log(`There are ${workerarray.length} workers`);
    }
  } catch (ex) {
    console.log(`Unexpected exception ${ex}`);
  }
};
const main = async () => {
  let svr = null;
  try {
    // console.log(
    //   `Validate the server can be instantiated with no parameters (e.g. new Server())`
    // );
    // svr = new Server();
    // console.log(`Verify default port is 7981`);
    // assert(svr.port === 7981, `Expected port to be 7981 but found ${svr.port}`);
    // console.log(`Verify default address is 127.0.0.1`);
    // assert(
    //   svr.address === "127.0.0.1",
    //   `Expected address to be '127.0.0.1' but found ${svr.address}`
    // );
    // console.log(`Verify max_connections = 5`);
    // assert(
    //   svr.max_connections === 5,
    //   `Expected max_connections to be 5 but found ${svr.max_connections}`
    // );
    // console.log(`Closing server`);
    // svr.close();

    // console.log(
    //   `Validate the server can be instantiated with only a port (e.g. new Server(3000))`
    // );
    // svr = new Server(3000);
    // console.log(`Verify the port is 3000`);
    // assert(svr.port === 3000, `Expected port to be 3000 but found ${svr.port}`);
    // console.log(`Verify the address is 127.0.0.1`);
    // assert(
    //   svr.address === "127.0.0.1",
    //   `Expected address to be '127.0.0.1' but found ${svr.address}`
    // );
    // console.log(`closing`);
    // svr.close();

    // console.log(
    //   `Validate the server can be instantiated with a port and an host (e.g. new Server(4000, "localhost"))`
    // );
    // svr = new Server(4000, "localHost");
    // console.log(`Verify the port is 4000`);
    // assert(svr.port === 4000, `Expected port to be 4000 but found ${svr.port}`);
    // console.log(`Verify the server is 127.0.0.1`);
    // assert(
    //   svr.address === "127.0.0.1",
    //   `Expected address to be '127.0.0.1' but found ${svr.address}`
    // );
    // svr.close();

    // console.log(
    //   `Validate the server can be instantiated with a port and an address (e.g. new Server(5000, '0.0.0.0'))`
    // );
    // svr = new Server(5000, "0.0.0.0");
    // console.log(`Verify the port is 5000`);
    // assert(svr.port === 5000, `Expected port to be 5000 but found ${svr.port}`);
    // let address = svr.address;
    // console.log(`Verify the address is 0.0.0.0`);
    // assert(
    //   svr.address.length === 7,
    //   `Expected svr.address.length to be 7 but found ${svr.address.length}`
    // );
    // for (let i = 0; i < address.length; i += 2) {
    //   assert(address[i] === "0", `Expected '0' but found "${address[i]}"`);
    //   if (i + 1 < address.length) {
    //     assert(
    //       address[i + 1] === ".",
    //       `Expected '.' but found "${address[i + 1]}"`
    //     );
    //   }
    // }
    // assert(
    //   svr.address == "0.0.0.0",
    //   `Expected address to be '0.0.0.0' but found [${svr.address}]`
    // );
    // svr.close();

    // console.log(
    //   `Validate the server can be instantiated with a port and an address (e.g. new Server(5000, '192.168.1.2'))`
    // );
    // svr = new Server(5000, "0.0.0.0");
    // console.log(`Verify the port is 5000`);
    // assert(svr.port === 5000, `Expected port to be 5000 but found ${svr.port}`);
    // address = svr.address;
    // console.log(`Verify the address is 0.0.0.0`);
    // assert(
    //   svr.address.length === 7,
    //   `Expected svr.address.length to be 7 but found ${svr.address.length}`
    // );
    // for (let i = 0; i < address.length; i += 2) {
    //   assert(address[i] === "0", `Expected '0' but found "${address[i]}"`);
    //   if (i + 1 < address.length) {
    //     assert(
    //       address[i + 1] === ".",
    //       `Expected '.' but found "${address[i + 1]}"`
    //     );
    //   }
    // }
    // assert(
    //   svr.address == "0.0.0.0",
    //   `Expected address to be '0.0.0.0' but found [${svr.address}]`
    // );
    // svr.close();

    // console.log(
    //   `Validate the server cannot be instantiated with a port and an invalid address (e.g. new Server(6000, '999.999.999.999'))`
    // );
    // try {
    //   svr = new Server(6000, "999.999.999.999");
    //   assert(svr == 1, "This cannot happen");
    // } catch (ex) {
    //   console.log(`Caught exception ${ex}`);
    //   assert(
    //     ex == "TypeError: Exception instantiating Server: Invalid host address",
    //     `Unexpected exception: ${ex}`
    //   );
    // }
    let message = "The listening callback was not invoked";
    // Listen tests
    console.log(`Validate the server can start workers`);
    const svr2 = await new Server();
    svr2.max_connections = 1;

    svr2
      .on("listening", async () => {
        console.log('listening callback');
        try {
          message = `The listening callback was invoked`;
          console.log(`this.constructor.name = ${svr2.constructor.name}`);

          await startworkers(svr2);
          os.setTimeout(() => {
            try {
              console.log(`This should cause the application to exit gracefully`);
              workerarray[0].postMessage({ type: "shutdown" });
              console.log(`Shutdown message sent`);
              svr2.close();
              console.log(`If you can see this then it passed`);
              console.log(`Unless the core dumps (sigh)`);
            } catch (ex) {
              console.log(`Exception in timeout ${ex}`, ex.stack);
            }
          }, 5000);
        } catch (ex) {
          console.log(`Caught exception in listening: ${ex}`);
        }
      })
      .listen();
    console.log(`Returned from listen`);
    assert(svr2.address == '127.0.0.1', `Expected address to be '127.0.0.1' but found [${svr2.address}]`);
    assert(message === "The listening callback was invoked", message);
  } catch (ex) {
    console.log(`Caught ${ex}`, ex.stack);
  }
};
console.log(`Beginning`);
main();
console.log(`Done`);
