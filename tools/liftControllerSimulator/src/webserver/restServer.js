const express = require("express");
const cors = require("cors");

const liftRouter = require("./routes/lift");
const settingsRouter = require("./routes/settings");
const uploadRouter = require("./routes/upload");

const restServer = express();

restServer.use(cors());
restServer.use(express.json());
restServer.use("/api/lift", liftRouter);
restServer.use("/api/settings", settingsRouter);
restServer.use("/api/upload", uploadRouter);

module.exports = restServer;