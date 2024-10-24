const express = require("express");
const router = express.Router();
const liftService = require("../../services/liftService");

router.route("/")
    .get((req, res) => 
    {
        res.status(200).send(liftService.settings);
    })
    .post((req, res) => 
    {
        const settings = req.body;

        if(!settings)
        {
            res.status(500).send();
            console.error("Setting settings failed");
            return;
        }

        liftService.settings = settings;
        res.status(200).send();

        console.log("Settings set to", settings);
    });

module.exports = router;