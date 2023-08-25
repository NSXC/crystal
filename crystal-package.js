const net = require('net');

class CrystalClient {
    constructor(host, port) {
        this.host = host || '127.0.0.1';
        this.port = port || 479;
    }

    async connectAndExecute(command) {
        return new Promise((resolve, reject) => {
            const client = new net.Socket();

            client.connect(this.port, this.host, () => {
                client.write(command);
            });

            client.on('data', (data) => {
                const responseData = data.toString();
                client.end();
                resolve(responseData);
            });

            client.on('close', () => {
                // Nothing specific to do here
            });

            client.on('error', (err) => {
                client.end();
                reject(err);
            });
        });
    }
}

module.exports = CrystalClient;
