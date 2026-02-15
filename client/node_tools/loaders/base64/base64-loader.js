
const fs = require('fs');

module.exports = function(content) {
    const resourcePath = this.resourcePath;
    const extension = resourcePath.split('.').pop();
    const base64 = Buffer.from(fs.readFileSync(resourcePath)).toString('base64');
    return `
        module.exports = 'data:image/${extension};base64,${base64}';
    `;
};
