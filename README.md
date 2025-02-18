# ADC-Conversor

## Descrição
Este projeto demonstra o uso de ADC, PWM e I2C com um display SSD1306 no Raspberry Pi Pico. Ele inclui:
- Leitura de valores do joystick usando ADC.
- Controle de brilho dos LEDs usando PWM.
- Comunicação com display SSD1306 via I2C.
- Manipulação de entradas de botões para alternar estados dos LEDs e funcionalidade PWM.

## Vídeo Demonstrativo
[![Demonstração do Projeto feito por Italo Cauã da Silva Santos](https://img.youtube.com/vi/e8URArvST00/0.jpg)](https://www.youtube.com/watch?v=e8URArvST00)

## Como Executar
1. Clone o repositório:
    ```sh
    git clone https://github.com/KayleKylian/ADC-Conversor.git
    ```
2. Navegue até o diretório do projeto:
    ```sh
    cd ADC-Conversor
    ```
3. Compile o código:
    ```sh
    mkdir build
    cd build
    cmake ..
    make
    ```
4. Carregue o binário no Raspberry Pi Pico.s

## Uso
- **Joystick**: Controla a posição de um quadrado no display e ajusta o brilho dos LEDs.
- **Botão JS (Joystick)**: Alterna o estado do LED verde e muda o tipo de borda do display.
- **Botão A**: Ativa ou desativa o controle PWM dos LEDs.

## Hardware Necessário
- Raspberry Pi Pico
- Display SSD1306
- Joystick
- LEDs (vermelho, azul, verde)
- Botões

## Conexões
- **I2C**: 
  - SCL: GPIO 15
  - SDA: GPIO 14
- **LEDs**:
  - Vermelho: GPIO 13
  - Azul: GPIO 12
  - Verde: GPIO 11
- **Botões**:
  - JS: GPIO 22
  - A: GPIO 5
- **Joystick**:
  - X: GPIO 26
  - Y: GPIO 27

## Autor
**Italo Cauã da Silva Santos**  
Engenheiro Mecânico

## Data
18 de fevereiro de 2025