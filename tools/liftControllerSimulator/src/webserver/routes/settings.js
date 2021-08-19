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
        res.status(501).send();
    });

module.exports = router;