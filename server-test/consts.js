const fs = require('fs');
const path = require('path');

module.exports.basePath = path.join(__dirname, '../src/index/build/esp32.esp32.esp32c3/');

module.exports.formatDateTime = function(date = new Date()) {
    const pad = (num) => String(num).padStart(2, '0');

    const day = pad(date.getDate());
    const month = pad(date.getMonth() + 1);
    const year = date.getFullYear();

    const hours = pad(date.getHours());
    const minutes = pad(date.getMinutes());
    const seconds = pad(date.getSeconds());

    return `${day}.${month}.${year} ${hours}:${minutes}:${seconds}`;
}