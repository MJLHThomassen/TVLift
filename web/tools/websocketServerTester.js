const http = require("http");
const WebSocket = require("ws");
const express = require("express");
const uuidv1 = require("uuid/v1");
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

  await sleep(200);

  send(ws);
}

function connection(ws)
{
  const id = uuidv1();
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
const port = 8081;

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

wss.on("connection", connection);

app.post("/api/lift/up", (req, res) => 
{
  res.status(200).send();
});

app.post("/api/lift/down", (req, res) => 
{
  res.status(200).send();
});

app.post("/api/lift/stop", (req, res) => 
{
  res.status(200).send();
});

app.post("/api/upload/firmware", (req, res) => 
{
  res.status(200).send();
});

server.listen(
  port,
  () => 
  {
    const isIPv6 = server.address().family === "IPv6";
    const host = (isIPv6 ? "[" : "") + server.address().address + (isIPv6 ? "]" : "");

    console.log("Server listening on", "http://" + host + ":" + port);
  }
);

