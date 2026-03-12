const crypto = require('crypto');
const path = require('path');

const authService = require('../service/auth.service');

const users = {}
const connections = {}

// Chamada feita pelo frontend pedindo o frame
exports.frame = (req, res) => {
    const { sessionId } = req.query;

    if (!sessionId) {
        return res.status(400).json({ erro: "Frontend não se identificou" });
    }

    const code = authService.getFrame(connections, sessionId);

    res.json({ frame: code });
}

// Cuida do SSE
exports.events = (req, res) => {
    const sessionId = req.query.sessionId;
    if (!sessionId) {
        return res.status(400).send('Session ID é obrigatório');
    }
    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection', 'keep-alive');

    users[sessionId] = res;
    console.log(`Novo cliente: ${sessionId}. Total de usuarios ativos: ${Object.keys(users).length}`);
    res.write(': ping\n\n');
    req.on('close', () => {
        delete users[sessionId];
        // To DO: Implementar logica para remover o relacionamento do token com o sessionId
        //delete connections[token];
        console.log(`Cliente ${sessionId} desconectado.`);
    });
}

// Requisicao feita pelo frontend para pegar o sessionId
exports.get_my_id = (req, res) => {
    const sessionId = crypto.randomBytes(8).toString('hex');
    res.json({ sessionId });
}

//Redireciona o usuario para a pagina de login
exports.login = (req, res) => {
    res.sendFile(path.join(__dirname, '../public/login.html'));
}

// Post feito pelo esp32 com o token assinado
exports.verify = (req, res) => {
    const { tok, chs, sig } = req.body;
    console.log(req.body);

    // Validacao basica de entrada
    if (tok === undefined || chs === undefined || !sig) {
        console.log("erro validação basica");
        return res.status(400).json({ erro: "Dados incompletos" });
    }

    const resultado = authService.verifyLogin(
        tok, chs, sig, users, connections
    );

    if (resultado.status === "INVALID") {
        console.log("Tentativa de login nao autorizda. Assinatura invalida.");
        return res.status(401).json({ erro: "Assinatura inválida" });
    }

    if (resultado.status === "NOT_FOUND") {
        console.log("Computador nao encontrado");
        return res.status(404).send("Computador não encontrado.");
    }

    res.status(200).send("Sucesso!");
}

exports.logado = (req, res) => {
    const absolutePath = path.join(process.cwd(), 'public', 'logado.html');
    res.sendFile(absolutePath);
}


