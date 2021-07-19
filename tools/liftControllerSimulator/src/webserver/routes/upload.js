const express = require("express");
const router = express.Router();

router.post("/firmware", (req, res) => 
{
    res.status(200).send();
});

module.exports = router;