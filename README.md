# Projeto: Estacione Fácil: Estacionamento Inteligente com Interface Web
**Fase 2 - EmbarcaTech**

## 📌 Descrição  
Este projeto implementa um **sistema de estacionamento inteligente** utilizando o microcontrolador **Raspberry Pi Pico W**, na placa **BitDogLab**.  
O sistema representa um estacionamento com 25 vagas, controladas por uma **matriz de LEDs WS2812**. Cada vaga pode ser reservada ou liberada por meio de uma **interface web**, e o status das vagas é exibido no **display OLED SSD1306**. A comunicação ocorre via Wi-Fi, com um servidor HTTP para interagir com os usuários.

## 🛠️ Componentes Utilizados  
- **Microcontrolador:** Raspberry Pi Pico W (Placa BitDogLab)  
- **Matriz de LEDs WS2812** (representação das vagas de estacionamento, controlada via PIO)  
- **Display OLED SSD1306** (via I2C, para exibição do status das vagas)  
- **Botões A e B** (entrada digital, botão B utilizado para BOOTSEL)  
- **Servidor HTTP** (comunicação via TCP para gerenciar reservas de vagas)  
- **Wi-Fi** (para conexão com rede local e acesso à interface web)

## 🔥 Funcionalidades  
- ✅ Interface web para visualizar e reservar vagas de estacionamento  
- ✅ Reserva e liberação de vagas, refletida na matriz de LEDs  
- ✅ Exibição do status das vagas no display OLED SSD1306  
- ✅ Conexão Wi-Fi via Raspberry Pi Pico W  
- ✅ Modo BOOTSEL acionável pelo botão B para reprogramação rápida  

## 📄 Estrutura do Projeto  
- `principal_iot.c` → Código principal para controle do estacionamento, servidor HTTP e interação com os periféricos  
- `ws2812.pio` → Programa PIO para controle da matriz de LEDs WS2812  
- `README.md` → Documentação do projeto  
- `Html/` → Arquivos da interface web (HTML)  
- `generated/` → Arquivos gerados para o controle da matriz de LEDs

## 🖥️ Como Executar o Projeto  

### Passo 1: Clone o repositório do projeto e abra-o no VS Code

### Passo 2: Configurar o Ambiente  
Garanta que o **Pico SDK** está corretamente instalado e que o **VS Code** possui a extensão **Raspberry Pi Pico**.

### Passo 3: Compilar o Código  
Compile o projeto pelo VS Code com a opção "Build".

### Passo 4: Carregar o Código na Placa  
1. Conecte a placa **BitDogLab** via cabo USB  
2. Coloque a placa em modo **bootsel** (pressione o botão B e conecte o USB)  
3. Use a opção "**Run Project (USB)**" da extensão para enviar o `.uf2`

### Passo 5: Conectar ao Wi-Fi  
1. Ao iniciar o sistema, o Raspberry Pi Pico W se conectará à rede Wi-Fi configurada no código (SSID e senha).  
2. Acesse a interface web no IP do Raspberry Pi Pico W para visualizar e reservar as vagas de estacionamento.

### Passo 6: Verificar o Funcionamento  
- A interface web permitirá que você veja o status de cada uma das 25 vagas.  
- Cada vez que uma vaga é reservada ou liberada, a matriz de LEDs será atualizada para refletir a mudança.  
- O display OLED pode mostrar informações adicionais, como número de vagas disponíveis.

## 📌 Autor  
Projeto desenvolvido por **Jeová Pinheiro** para a fase 2 do ***EmbarcaTech***
