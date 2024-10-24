const WebSocket = require("ws");
const uuid = require("uuid");
const Ansicolor = require("ansicolor");
const dayjs = require("dayjs");

const levels = ["V", "D", "I", "W", "E"];

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

    const date = dayjs().format("YYYY-MM-DD HH:mm:ss");
    const level = levels[Math.round(Math.random() * 4)];
    ws.send(`${level} (${date}) This is a log message\n`);

    await sleep(Math.random() * 1000 + 500);

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