const frameManager = require('../frameManager');

// Verifica os dados vindo do esp32
exports.verifyLogin = (tok, chs, sig, user, connections) => {
    // Junta o Token com o checksum
    const bufferDados = Buffer.from([parseInt(tok), parseInt(chs)]);

    // Valida a assinatura
    const autorizado = frameManager.validateSignature(sig, bufferDados);

    if (!autorizado) {
        return { status: "INVALID" };
    }

    console.log(`Login autorizado para o Token ${tok}`);

    // Pega o sessionId associado ao Token, se nao existir retorna undefined
    const sessionIdReal = connections[parseInt(tok)];

    console.log(sessionIdReal);

    // Se o sessionId for string valida, e se clientes[sessionIdReal] nao retorna undefined
    // Passa se existir sessionId e se existir cliente com este sessionId
    if (sessionIdReal && user[sessionIdReal]) {
        console.log(`Sucesso! Redirecionando o frontend ${sessionIdReal}`);

        user[sessionIdReal].write(`data: ${JSON.stringify({ action: 'redirect', url: '/logado' })}\n\n`);

        delete connections[parseInt(tok)];
        delete user[sessionIdReal];

        return { status: "OK"}
    }
    return { status: "NOT_FOUND"}

};

// Gera e retorna o frame
exports.getFrame = (connections, sessionId) => {
    const bufferToken = frameManager.generateToken();
    const code = frameManager.generateFrame(bufferToken);
    
    // Imprime para debug
    const token = bufferToken[0];
    console.log(`Ponte criada: Token=${token} -> Frontend=${sessionId}`);

    // Relacionamos o sessionId ao token
    connections[token] = sessionId;
    
    return code;
};