const CrystalClient = require('./cacheClient');

async function main() {
    const client = new CrystalClient('127.0.0.1', 479);

    console.log("getting value");

    try {
        const username = await client.connectAndExecute("GET:user123");
        console.log('Response from GET:', username);
    } catch (error) {
        console.error('Error:', error);
    }
}

main();
