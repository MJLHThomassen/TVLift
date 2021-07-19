const express = require("express");
const router = express.Router();
const liftService = require("../../services/liftService");

router.get("/status", (req, res) => 
{
    res.status(200).send({ status: liftService.status });
});

router.post("/up", (req, res) => 
{
    res.status(200).send();
});

router.post("/down", (req, res) => 
{
    res.status(200).send();
});

router.post("/stop", (req, res) => 
{
    res.status(200).send();
});

module.exports = router;