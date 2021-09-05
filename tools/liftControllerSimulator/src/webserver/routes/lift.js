const express = require("express");
const router = express.Router();
const liftService = require("../../services/liftService");

router.get("/status", (req, res) => 
{
    res.status(200).send({ status: liftService.status });
});

router.route("/speed")
    .get((req, res) => 
    {
        res.status(200).send({ speed: liftService.speed });
    })
    .post((req, res) => 
    {
        const speed = req.body?.speed;

        if(!speed)
        {
            res.status(500).send();
            console.error("Setting speed failed");
            return;
        }

        liftService.speed = speed;
        res.status(200).send();

        console.log("Speed set to", speed);
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