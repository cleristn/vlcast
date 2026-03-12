const cripto = require('crypto');
const { calculateCRC8 } = require('./calculateCRC8');
const fs = require('fs');

const SOF = '01011'; // Bits que o esp32 identifica como inicio

function validateSignature(assinaturaHex, bufferDados) {
    try {
        //Converter a assinatura que vem em HEX (String) para Buffer (Binário)
        const assinaturaBuffer = Buffer.from(assinaturaHex, 'hex');

        // Verificar
        const isValid = cripto.verify(
            'SHA256',
            bufferDados,
            fs.readFileSync('public_key.pem', 'utf8'),
            assinaturaBuffer
        );

        return isValid;
    } catch (e) {
        console.error("Erro na validação:", e.message);
        return false;
    }
}

function generateToken() {
    const bufferToken = cripto.randomBytes(1);

    // A IMPLEMENTAR:
    // salvar no banco de dados
    // e fazer verificacao sempre para nao repetir

    return bufferToken;
}

function generateChecksum8(buffer) {

    const crcValor = calculateCRC8(buffer);

    // Converte para binário, garante 8 caracteres e preenche com 0
    // O toString(2) converte para base 2
    // O padStart(8, '0') garante que o resultado tenha sempre 8 dígitos
    return crcValor.toString(2).padStart(8, '0');
}

function generateFrame(bufferToken) {
    const byte = bufferToken[0];
    const token = byte.toString(2).padStart(8, '0');
    const checksum = generateChecksum8(bufferToken);

    const finalCode = SOF + token + checksum;
    console.log(finalCode);
    return finalCode;
}

module.exports = {
    generateFrame,
    generateToken,
    validateSignature,
};