function calculateCRC8(data) {
    let crc = 0x00;

    for (let i = 0; i < data.length; i++) {
        crc ^= data[i];

        for (let j = 0; j < 8; j++) {
            if ((crc & 0x80) !== 0) {
                // Aplica o deslocamento e o polinômio 0x07
                // O ">>> 0" garante que o JS trate como unsigned int de 32 bits antes do truncamento
                crc = ((crc << 1) ^ 0x07) & 0xFF;
            } else {
                crc = (crc << 1) & 0xFF;
            }
        }
    }
    return crc;
}

module.exports = { calculateCRC8 };