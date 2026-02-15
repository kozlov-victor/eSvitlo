
const fs = require('fs');
const path = require('path');

class ResourceHeader {
    constructor() {
        this.name = '';
        this.size = 0;
    }
}

class ResourcePacker {

    constructor() {
        this.headers = [];
        this.body = [];
    }

    #uint32ToBytes(int) {
        if (int<0) throw new Error(`bad uint value: ${int}`)
        return [
            (int >> 24) & 0xff,
            (int >> 16) & 0xff,
            (int >> 8) & 0xff,
            int & 0xff,
        ]
    }

    #stringToBytes(str) {
        return str.split('').map(it=>it.codePointAt(0));
    }

    addResource(name, url) {
        const buffer = fs.readFileSync(url);
        const length = buffer.length;
        const header = new ResourceHeader();
        header.name = name;
        header.size = length;
        this.headers.push(header);
        for (const bufferElement of buffer) {
            this.body.push(bufferElement);
        }
    }

    addResourceDir(url) {
        const files = fs.readdirSync(url);
        files.forEach(file=>{
            this.addResource(file, path.join(url, file));
        });
    }

    build(fileName) {
        const out = [];
        out.push(...this.#stringToBytes('vEnginePkg'));
        out.push(...this.#uint32ToBytes(this.headers.length));
        for (const header of this.headers) {
            out.push(...[...this.#stringToBytes(header.name),0]);
            out.push(...this.#uint32ToBytes(header.size));
        }
        for (const bodyElement of this.body) {
            out.push(bodyElement);
        }
        fs.writeFileSync(fileName, new Uint8Array(Buffer.from(out)));
    }

}

module.exports = ResourcePacker;
