const WebSocket = require("ws");
const uuid = require("uuid");
const Ansicolor = require("ansicolor");

function sleep(ms)
{
    return new Promise((resolve) => setTimeout(resolve, ms));
}

async function send(ws)
{
    if (ws.readyState === WebSocket.CLOSED)
    {
        return;
    }

    const date = Date();
    ws.send(date);

    await sleep(2000);

    send(ws);
}

function connection(ws)
{
    const id = uuid.v1();

    console.log("Connected", id);

    ws.send(Ansicolor.red(id));

    ws.onmessage = message => 
    {
        console.log(id, "Message: ", message.data);
    };

    ws.onclose = close =>
    {
        console.log("Closed", id);
    };

    send(ws);
}

function createWebSocketServer(server)
{
    const wss = new WebSocket.Server({ server });
    wss.on("connection", connection);
    return wss;
}

module.exports = createWebSocketServer;