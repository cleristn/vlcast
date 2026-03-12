const crypto = require('crypto');
const fs = require('fs');

function cryptoStart() {
    // Gera as chaves
    const { publicKey, privateKey } = crypto.generateKeyPairSync('ec', {
        namedCurve: 'prime256v1', 
        publicKeyEncoding: { type: 'spki', format: 'pem' },
        privateKeyEncoding: { type: 'pkcs8', format: 'pem' }
    });

    // Define o nome dos arquivos
    const publicFile = 'public_key.pem';
    const privateFile = 'private_key.pem';

    // Salva as chaves no disco
    fs.writeFileSync(publicFile, publicKey);
    fs.writeFileSync(privateFile, privateKey);

    console.log('Chaves geradas com sucesso!');
    console.log(`Public Key salva em  : ${publicFile} (Deixe no Backend)`);
    console.log(`Private Key salva em : ${privateFile} (Copie para o firmware ESP32)`);
}

cryptoStart();