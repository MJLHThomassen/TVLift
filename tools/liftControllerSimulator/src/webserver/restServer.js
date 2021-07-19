const express = require("express");
const cors = require("cors");

const liftRouter = require("./routes/lift");
const uploadRouter = require("./routes/upload");

const restServer = express();

restServer.use(cors());
restServer.use("/api/lift", liftRouter);
restServer.use("/api/upload", uploadRouter);

module.exports = restServer;