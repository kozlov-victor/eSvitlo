const fs = require('fs');
const basePath = require('./consts').basePath;
const formatDateTime = require('./consts').formatDateTime;

const buffer = fs.readFileSync(basePath + '/index.ino.bin');

// Перетворюємо в ASCII-рядок
const text = buffer.toString('latin1'); // важливо не utf8

const match = text.match(/FW_VERSION=([0-9.]+)/);

if (!match) {
    console.error("Version not found in firmware.bin");
    process.exit(1);
}

const version = match[1];

console.log("Detected firmware version:", version);

fs.writeFileSync(
    basePath + '/version.json',
    JSON.stringify({
        version: version,
        created: formatDateTime()
    }, null, 4)
);