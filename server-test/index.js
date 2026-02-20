const http = require('http');
const fs = require('fs');
const os = require('os');

const PORT = 8080;
const basePath = require('./consts').basePath;
const formatDateTime = require('./consts').formatDateTime;

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

const createResponse = (req,res,fileUrl,contentType)=>{

    const user = req.headers['x-esvitlo-app'];

    if (!user) {
        res.writeHead(403);
        return res.end('Not allowed');
    }

    if (!fs.existsSync(fileUrl)) {
        res.writeHead(404);
        return res.end('Firmware not found');
    }

    const stat = fs.statSync(fileUrl);

    res.writeHead(200, {
        'Content-Type': contentType,
        'Content-Length': stat.size,
        'Connection': 'close'
    });

    const readStream = fs.createReadStream(fileUrl);

    readStream.pipe(res);

    readStream.on('end', () => {
        res.end();
    });
}

const server = http.createServer((req, res) => {
    console.log(req.url);
    if (req.url === '/upgrade') {
        console.log('Firmware requested');
        createResponse(req,res,basePath + 'index.ino.bin','application/octet-stream');
    }
    else if (req.url==="/update") {
        createResponse(req,res,basePath + 'version.json','application/json');
    }
    else {
        res.writeHead(200);
        res.end(JSON.stringify({status:'OK',time: formatDateTime()}));
    }
});

server.listen(PORT, '0.0.0.0', () => {
    const ips = getLocalIPs();
    console.log('dev Server started.');
    ips.forEach(ip => {
        console.log(`Use this URL on ESP32: http://${ip}:${PORT}`);
    });
});