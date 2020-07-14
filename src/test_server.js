import { Server } from '/usr/local/lib/libqjsnet.so';

const assert = (b, str) => {
    if (b) {
        return;
    } else {
        throw Error("assertion failed: " + str);
    }
}

class ColorPoint extends Server {
    constructor(x, y, color) {
        super(x, y);
        this.color = color;
    }
    get_color() {
        return this.color;
    }
};

const main = () => {
  console.log (`This is main`);
    let pt = null;
    let pt2 = null;

    pt = new Server(2, 3);
    assert(pt.x === 2);
    assert(pt.y === 3);
    pt.x = 4;
    assert(pt.x === 4);
    assert(pt.norm() == 5);

    pt2 = new ColorPoint(2, 3, 0xffffff);
    assert(pt2.x === 2);
    assert(pt2.color === 0xffffff);
    assert(pt2.get_color() === 0xffffff);
    console.log(`If you can see this then it passed`);
}
console.log(`Beginning`);
main();
console.log(`Done`);
