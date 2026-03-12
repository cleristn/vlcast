const express = require('express');
const router = express.Router();

const authController = require('../controllers/auth.controller')

router.get('/frame', authController.frame);

router.get('/login', authController.login);

router.get('/events', authController.events); 

router.get('/get-my-id', authController.get_my_id);

router.post('/verify', authController.verify);

router.get('/logado', authController.logado);

module.exports = router;