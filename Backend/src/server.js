const express = require('express');
const path = require('path');

const app = express();

const router = require('./routes/auth.routes.js');

app.use(express.json());
app.use('/', router);

app.use(express.static('public'));

app.listen(3000, () => {
    console.log('Servidor iniciando...');
})
