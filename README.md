# VLCast
Visual Light Communication system. Atualmente focado em autenticação.

>_Nota: este projeto usa piscadas rápidas na tela — veja o aviso de fotossensibilidade antes de testar._
## Descrição

VLCast é um sistema EXPERIMENTAL de Prova de Conceito (PoC) que usa conceitos de _Visible Light Communication (VLC)_ e _Out-of-Band Authentication (OOBA)_. Ele permite autenticação em sites sem a necessidade de digitar login e senha, utilizando apenas a luz do monitor e um ESP32 equipado com um sensor de luz.

## Motivação

Explorar uma alternativa experimental que tenta resolver o problema de memorização múltiplas senhas, e também permitir a possibilidade de logar em computadores desconhecidos sem a necessidade de digitar senha ou login, evitando o risco de intercepção por keyloggers. O projeto foi desenvolvido estritamente como uma Prova de Conceito (PoC).

## Como Executar
#### ⚠️ Aviso sobre Fotossensibilidade

>O VLCast utiliza piscadas rápidas na tela para transmissão de dados. Pessoas com fotossensibilidade devem evitar utilizar o sistema.
>A ausência de vídeo demonstrativo se deve a esse fator. Em versões futuras, com um sensor óptico mais sensível, a intensidade das piscadas poderá ser reduzida.

### Backend

Crie suas chaves pública e privada, estando dentro da pasta `src` do Backend com o seguinte comando:

```
node keygen.js
```

Isto irá gerar dois arquivos: `public_key.pem` e `private_key.pem`

Copie o `private_key.pem` para `firmware/main/certs/`

Depois, navegue até a pasta do backend e instale as dependências:

```
npm install
node server.js
```

O servidor iniciará na porta 3000.

### Firmware (ESP32)

Navegue até a pasta do firmware.
Antes de compilar, certifique-se de configurar as suas credenciais de Wi-Fi e IP do servidor com o comando:

```
idf.py menuconfig
```

Compile e grave utilizando o ESP-IDF:
```
idf.py build
idf.py -p (SUA_PORTA) flash monitor
```

## Como funciona

O fluxo de autenticação ocorre na seguinte sequência:

1. **Requisição:** Ao abrir a página de login, o frontend solicita um Token à API.
    
2. **Transmissão Visual:** A API retorna o Token, e o frontend converte esses bits em uma sequência de piscadas no monitor.
    
3. **Captura:** O usuário aponta o sensor do ESP32 para a tela. O dispositivo detecta o código de início e armazena os bits seguintes (Token + checksum).
    
4. **Assinatura:** O ESP32 assina o Token e o checksum utilizando sua chave privada e os envia para a API (via backend).
    
5. **Validação:** A API, que possui a chave pública do dispositivo, valida a assinatura.
    
6. **Autorização:** A API confirma o login e avisa o frontend via Server-Sent Events (SSE) usando o `sessionId` criado no início da conexão  para redirecionar a página. A unicidade do Token e a identificação do ESP32 garantem quem realizou o login.

## Arquitetura

O VLCast utiliza um ESP32 com firmware escrito em C utilizando o framework da Espressif (ESP-IDF) e um LED com capacitores cerâmicos, para captar variações de luminosidade da tela do monitor, que representa bits por meio de piscadas controladas.

O ESP32 utiliza o LED como um fotodiodo improvisado para capturar a variação da luz. E para mitigar a interferência da rede elétrica (60 Hz), o microcontrolador realiza amostras em um intervalo de 16.677 ms, para conseguir anular a interferência da rede elétrica. 

A API desenvolvida com Node.js gera o Token e envia para o frontend, que converte o frame em uma sequência de piscadas na tela. O Token é capturado, interpretado e assinado pelo ESP32 utilizando sua chave privada, sendo então enviado para a API para validação da identidade e autorização do login.

A comunicação entre frontend e o backend é feita via Server-Sent Events (SSE), pois o backend precisa apenas notificar o frontend após receber a resposta do ESP32. Alternativas como WebSocket ou Long Polling seriam pesadas e ineficientes para este proposito de conexão curta, e unidirecional. 

Atualmente, toda a comunicação ocorre via HTTP para fins de depuração. Em um ambiente de produção, a utilização de HTTPS é obrigatória.

<img width="726" height="189" alt="Frame" src="https://github.com/user-attachments/assets/c5426a03-ef85-4fad-a339-9659a519c337" />

O frame neste modelo contém 21 bits no total.

## Tecnologias Utilizadas

- Hardware: ESP32S3, LED (como fotodiodo improvisado), 2 capacitores cerâmicos de 470 nF
- Firmware: C/ESP-IDF
- Backend: Node.js, Express
- Criptografia: mbedtls (ESP32), módulo `crypto` nativo (Node.js)
## Segurança

Por se tratar de um protótipo, o sistema possui vulnerabilidades conhecidas:

- Não possui proteção contra _Replay Attacks_.
- Ausência de HTTPS no ambiente atual.
- A chave privada precisa de um armazenamento físico mais seguro no hardware.
- Risco de "Token Falso": O usuário não possui uma tela no dispositivo físico para confirmar o que está assinando antes da autenticação.

## Limitações

A primeira limitação é a capacidade de leitura do LED que está sendo usado como fotodiodo. Ele não é sensível a pequenas variações de luz e, devido ao ruído da rede elétrica, foi necessário utilizar a media do sinal coletado num intervalo de 16.677 ms, o que reduz consideravelmente a velocidade e a quantidade de bits transmitidos.

Devido a essa limitação, o frontend limita a duração do pulso a 150 ms, o que reduz a transmissão para aproximadamente 6 bits por segundo. Como o sistema atual possui 21 bits no total, a transmissão leva teoricamente cerca de 3,2 segundos, porém na prática pode levar mais que o dobro disso por causa da dificuldade de identificação causada pelos ruídos, especialmente quando o sensor é colocado na tela após já ter passado o código que identifica o início.

Existem outras limitações atuais no projeto, como o tamanho fixo do frame e o fato do sensor ser sensível a ruído elétrico. Ainda não foi implementada a expiração do Token, nem a verificação de duplicidade em tempo real. Além disso, o tamanho do Token limita o sistema a suportar apenas 256 usuários simultaneamente.

Também ainda não implementada uma forma de registrar dinamicamente o dispositivo na API, sendo suportado apenas um dispositivo atualmente. Há necessidade de implementar um banco de dados para armazenar os dispositivos cadastrados de forma persistente, pois no modelo atual ele suporta apenas um único dispositivo. E armazenar o Token usado na autenticação, para ele não se repetir.

Para maior robustez do sistema, também é necessário implementar métodos de checksum mais confiáveis. A forma como a API relaciona o frontend com o Token poderia ser feita utilizando algum armazenamento temporário mais seguro.

No modelo de autenticação, há ainda a necessidade de implementar uma pequena tela no dispositivo, para que o usuário visualize o que está assinando, evitando ataques de falso Token.

## Decisões de Projeto

#### Por que tamanho fixo de frame inicialmente?
Para facilitar debug. Por se tratar de um protótipo, o objetivo principal era validar o funcionamento do conceito.
#### Por que CRC8 e não outro método?
Devido à extrema lentidão na transferência de bits causada pela ineficiência do LED como fotodiodo. O CRC8 utiliza apenas 8 bits, sendo mais rápido de transmitir.
#### Por que 16.677 ms?
A rede elétrica oscila em 60 Hz, gerando ruído na iluminação do ambiente a cada 16.677 ms. O ESP32 captura o sinal nesse intervalo exato e faz uma média simples, fazendo a interferência da onda desaparecer da leitura.
#### Por que o ESP32?
É um sistema consolidado, eficiente para leitura de sensores em tempo real e excelente para prototipação rápida.
#### Por que Node.js?
É um ecossistema moderno, ágil para criação de APIs e atende perfeitamente à necessidade de orquestração de requisições assíncronas (como o SSE).
#### Por que usar um LED como fotodiodo e como ele foi adaptado?
Foi uma adaptação necessária pela falta de componentes ópticos específicos, mas cumpriu o papel de provar que o conceito funciona. Para conseguir captar a luz da melhor forma possível, utilizei um **LED RGB**, mas conectando apenas o pino da **cor vermelha**, e os restantes dos pinos no GND. Além disso, precisei "planar" (lixar) a cabeça arredondada do LED para que a luz incidisse de forma mais reta, cobri todo o corpo dele com fita isolante para bloquear a interferência de luzes do ambiente externo, e coloquei papel alumínio internamente na tentativa de refletir e direcionar melhor a luz para o sensor (embora eu não tenha como medir se o alumínio teve um impacto realmente positivo).

![Hardware](https://github.com/user-attachments/assets/e66a6b2f-c0e7-4ee7-bdf3-9e1a0d826546)

Foto do LED na protoboard.

![Signals](https://github.com/user-attachments/assets/4baf9e9f-1d0a-4bb1-819e-6e9545ffa426)

Print do gráfico no SerialPlotter representando o sinal emitido pela luz sobre o LED.
Neste sinal o SOF representa 01011, o Token 01010000 e o Checksum representa 10110111.

>_Nota: No código contem alguns printf espalhado apenas para fins de debug._

## Roadmap Futuro

- Substituir o LED por um fotodiodo especializado para capturar variações sutis de luz, o que exigirá reescrever a interpretação de sinal.
- Implementar banco de dados para gestão de múltiplos dispositivos e expiração de tokens.
- Refatoração de código e implementação de documentação técnica.
- Corrigir as falhas de segurança (implementar HTTPS, proteção contra _replay attacks_ e visor de confirmação no hardware).    
- Expandir a aplicação: utilizar a arquitetura baseada em luz para criar uma "caneta óptica" capaz de detectar posições na tela, funcionando como um cursor touch alternativo.
