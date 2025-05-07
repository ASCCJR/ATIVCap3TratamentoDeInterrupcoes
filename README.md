# ATIVCap3TratamentoDeInterrupcoes

# Detecção de Som com Matriz Neopixel (Raspberry Pi Pico)

Este projeto demonstra como usar o Raspberry Pi Pico para detectar sons através de um microfone analógico e controlar uma matriz de LEDs Neopixel com base na intensidade do som detectado. Quando o nível de som captado pelo microfone ultrapassa um determinado limiar, todos os LEDs da matriz acendem em branco. Abaixo desse limiar, os LEDs permanecem apagados.

## Visão Geral

O código realiza as seguintes etapas:

1.  **Inicialização:** Configura o Standard I/O para comunicação serial (via USB ou UART, dependendo da configuração do CMakeLists.txt).
2.  **Configuração do ADC:** Inicializa o periférico ADC e habilita o pino GPIO conectado ao microfone como entrada analógica.
3.  **Configuração do PIO para Neopixels:** Utiliza o Programmable I/O (PIO) para controlar eficientemente a matriz de LEDs Neopixel, enviando os dados de cor necessários.
4.  **Configuração do Timer Periódico:** Cria um timer de hardware que chama uma função de callback periodicamente para ler o valor do microfone.
5.  **Detecção de Som:** Na função de callback do timer, o valor analógico do microfone é lido através do ADC. Este valor é comparado a um limiar predefinido (`SOUND_THRESHOLD`).
6.  **Controle dos Neopixels:**
    * Se o valor do ADC exceder o limiar, a função `fill_matrix()` é chamada para acender todos os LEDs da matriz em branco.
    * Se o valor do ADC estiver abaixo do limiar, a função `fill_matrix()` é chamada para apagar todos os LEDs da matriz (cor preta).
7.  **Loop Principal:** O programa entra em um loop infinito, com a lógica principal de leitura do microfone e controle dos LEDs sendo executada na função de callback do timer.

## Notas

* **`SOUND_THRESHOLD`:** Este valor define a sensibilidade à detecção de som. Você **precisará ajustar este valor** com base nas características do seu microfone e no nível de ruído ambiente. Monitore os valores do ADC no Serial Monitor (ativado no `stdio_init_all()`) em repouso e quando houver som para encontrar um limiar adequado. Um valor inicial baixo como 2100 pode precisar ser aumentado significativamente.
* Para projetos maiores com muitos Neopixels, é crucial usar uma fonte de alimentação externa para evitar sobrecarregar a porta USB do Raspberry Pi Pico.
* O código inclui `printf` para depuração, que envia informações para o Serial Monitor. Conecte um terminal serial (ex: usando `screen /dev/ttyACM0 115200` no Linux ou PuTTY no Windows) para visualizar essas mensagens.
