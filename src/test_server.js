import { AF_INET, AF_INET6, Server } from '/usr/local/lib/libqjsnet.so';

console.log(`AF_INET is ${AF_INET}`);
console.log(`AF_INET6 is ${AF_INET6}`);

const assert = (b, str) => {
    if (b) {
        return;
    } else {
        throw Error("assertion failed: " + str);
    }
}

// class ColorPoint extends Server {
//     constructor(x, y, color) {
//         super(x, y);
//         this.color = color;
//     }
//     get_color() {
//         return this.color;
//     }
// };

const main = () => {
    console.log(`Validate the server can be instantiated with no parameters (e.g. new Server())`);
    let svr = new Server();
    assert(svr.port === 7981, `Expected port to be 7981 but found ${svr.port}`);
    assert(svr.type === AF_INET6, `Expected ip type to be AF_INET6 but found ${svr.type}`);
    assert(svr.address === "localhost", `Expected address to be 'localhost' but found ${svr.address}`);

    console.log(`Validate the server can be instantiated with only a port (e.g. new Server(3000))`);
    svr = new Server(3000);
    assert(svr.port === 3000, `Expected port to be 3000 but found ${svr.port}`);
    assert(svr.type === AF_INET6, `Expected ip type to be AF_INET6 but found ${svr.type}`);
    assert(svr.address === "localhost", `Expected address to be 'localhost' but found ${svr.address}`);

    console.log(`Validate the server can be instantiated with a port and an IP type (e.g. new Server(4000, AF_INET))`);
    svr = new Server(4000, AF_INET);
    assert(svr.port === 4000, `Expected port to be 4000 but found ${svr.port}`);
    assert(svr.type === AF_INET, `Expected ip type to be AF_INET but found ${svr.type}`);
    assert(svr.address === "localhost", `Expected address to be 'localhost' but found ${svr.address}`);

    console.log(`Validate the server can be instantiated with a port and an IP type and an address (e.g. new Server(5000, AF_INET, '0.0.0.0'))`);
    svr = new Server(5000, AF_INET, "0.0.0.0");
    assert(svr.port === 5000, `Expected port to be 5000 but found ${svr.port}`);
    assert(svr.type === AF_INET, `Expected ip type to be AF_INET but found ${svr.type}`);
    const address = svr.address;
    assert(svr.address.length === 7, `Expected svr.address.length to be 7 but found ${svr.address.length}`);
    for (let i = 0; i < address.length; i+=2) {
        assert(address[i] === '0', `Expected '0' but found "${address[i]}"`);
        if (i + 1 < address.length) {
            assert(address[i+1] === '.', `Expected '.' but found "${address[i+1]}"`);
        }
    }
    // TODO: Understand why this does not work.  Probably over doing it with AF_INET and useip
    assert(svr.address == '0.0.0.0', `Expected address to be '0.0.0.0' but found [${svr.address}]`);

    console.log(`If you can see this then it passed`);
}
console.log(`Beginning`);
main();
console.log(`Done`);
