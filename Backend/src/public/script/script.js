const codeBox = document.querySelector('#codeBox')

let i = 0;
let frame;
let lastTime = 0;
const interval = 150; //ms 539 375
let code;

// Faz um request pedindo o frame
// e passa como parametro via query string o ID da sessao
async function getFrame(id) {
    try {
        const response = await fetch('/frame?sessionId=' + id);
        const data = await response.json();
        frame = data.frame;
    } catch (erro) {
        console.log('Erro na comunicação', erro);
    }
}

// Inverte a cor de acordo com o bit 0 ou 1
function updateScreen() {
    // 1 = Branco, 0 = Preto
    if (code[i] === 1) {
        codeBox.classList.remove('black');
        codeBox.classList.add('white');
    } else {
        codeBox.classList.remove('white');
        codeBox.classList.add('black');
    }
}

function loadCodeBox(now) {
    if (!lastTime) lastTime = now;

    const delta = now - lastTime;

    if (i >= code.length) {
        i = 0;
    }

    if (delta >= interval) {
        if (i >= code.length) {
            i = 0;
        }
        updateScreen();
        i++;
        lastTime = now - (delta % interval);
    }
    requestAnimationFrame(loadCodeBox);
}

async function startSSE() {
    // Pede um ID ao servidor
    const response = await fetch('/get-my-id');
    const { sessionId } = await response.json();

    console.log("Meu ID de sessão é:", sessionId);

    // Conecta ao SSE usando o ID recebido
    const evs = new EventSource(`/events?sessionId=${sessionId}`);

    evs.onmessage = (event) => {
        const data = JSON.parse(event.data);
        if (data.action === 'redirect') {
            window.location.href = data.url;
        }
    };

    return sessionId;
}

async function main() {
    const id = await startSSE();

    await getFrame(id);
    if (!frame) {
        console.error('Frame inválido');
        return;
    }
    code = frame.split('').map(Number);
    lastTime = performance.now();
    requestAnimationFrame(loadCodeBox);
}

// Chama a funcao main apos o DOM carregar
document.addEventListener('DOMContentLoaded', main);