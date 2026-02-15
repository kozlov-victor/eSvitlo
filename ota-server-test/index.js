const http = require('http');
const fs = require('fs');
const path = require('path');
const os = require('os');

const PORT = 8080;
const firmwarePath = path.join(__dirname, 'index.ino.bin');

function getLocalIPs() {
    const interfaces = os.networkInterfaces();
    const results = [];

    for (const name of Object.keys(interfaces)) {
        for (const net of interfaces[name]) {
            if (net.family === 'IPv4' && !net.internal) {
                results.push(net.address);
            }
        }
    }
    return results;
}

const server = http.createServer((req, res) => {
    if (req.url === '/firmware.bin') {
        console.log('Firmware requested');

        if (!fs.existsSync(firmwarePath)) {
            res.writeHead(404);
            return res.end('Firmware not found');
        }

        const stat = fs.statSync(firmwarePath);

        res.writeHead(200, {
            'Content-Type': 'application/octet-stream',
            'Content-Length': stat.size
        });

        const readStream = fs.createReadStream(firmwarePath);
        readStream.pipe(res);

    } else {
        res.writeHead(200);
        res.end('OTA Server Running');
    }
});

server.listen(PORT, () => {
    const ips = getLocalIPs();
    console.log('OTA Server started.');
    ips.forEach(ip => {
        console.log(`Use this URL on ESP32: http://${ip}:${PORT}/firmware.bin`);
    });
});