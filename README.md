## Projeto para Raspberry Pi Pico que detecta níveis de som através de um microfone analógico e exibe uma animação visual responsiva em uma matriz de LEDs WS2812 (Neopixel).

## 📋 Requisitos Atendidos

Este projeto foi desenvolvido com o objetivo de cumprir os seguintes requisitos:

- ✅ **Leitura do microfone via ADC:** Captura o sinal de áudio analógico através do conversor Analógico-Digital (ADC) do Raspberry Pi Pico.
- ✅ **Uso de timer periódico com callback:** Utiliza um timer configurado para disparar periodicamente, garantindo leituras consistentes do microfone.
- ✅ **Estabelecimento de limiar de detecção de som:** Define um valor de limiar (threshold) para identificar sons considerados "altos".
- ✅ **Controle da matriz WS2812:** Comanda os LEDs da matriz Neopixel para exibir padrões de luz.
- ✅ **Animação quando o limiar é ultrapassado:** Ativa uma animação visual específica na matriz de LEDs apenas quando o nível de som detectado excede o limiar configurado.

## 💡 Funcionalidades

  * **Detecção de Som:** Monitora continuamente o nível de som ambiente.
  * **Animação Responsiva:** Quando um som alto é detectado, uma animação de "onda" se propaga do centro da matriz.
  * **Visualização Dinâmica:** A cor e intensidade da animação são influenciadas pelo nível do som detectado.
  * **Eficiência:** Utiliza o PIO (Programmable I/O) do Pico para um controle eficiente dos LEDs WS2812.
  * **Configurável:** O limiar de som, pinos e parâmetros da matriz podem ser ajustados facilmente no código.

## 🔌 Conexões

As conexões são baseadas nas definições (`#define`) no código:

  * **Microfone Analógico:**
      * Saída Analógica do Microfone ➡️ **GPIO 28 (ADC2)** do Raspberry Pi Pico.
      * GND do Microfone ➡️ **GND** do Raspberry Pi Pico.
      * VCC do Microfone ➡️ **3.3V** do Raspberry Pi Pico.
  * **Matriz de LEDs WS2812:**
      * Data IN da Matriz WS2812 ➡️ **GPIO 7** do Raspberry Pi Pico.
      * GND da Matriz WS2812 ➡️ **GND** do Raspberry Pi Pico (**importante conectar ao mesmo GND do Pico**).
      * VCC da Matriz WS2812 ➡️ **3.3V** do Raspberry Pi Pico **OU** uma **Fonte de Alimentação Externa** adequada (lembre-se do GND comum).

## ⚙️ Configuração do Código

Pode ajustar o comportamento do projeto modificando as definições (`#define`) no início do arquivo `main.c`:

  * `AUDIO_INPUT_PIN`: Pino GPIO conectado à saída analógica do microfone (padrão: `28`).
  * `AUDIO_ADC_CHANNEL`: Canal do ADC correspondente ao pino GPIO (padrão: `2`).
  * `NEOPIXEL_PIN`: Pino GPIO conectado ao DATA IN da matriz Neopixel (padrão: `7`).
  * `NUM_PIXELS`: Número total de LEDs na sua matriz (padrão: `25` para 5x5).
  * `MATRIX_WIDTH`, `MATRIX_HEIGHT`: Dimensões da sua matriz (padrão: `5`, `5`).
  * `SOUND_THRESHOLD`: **Limiar de detecção de som.** Este valor (entre 0 e 4095, correspondendo à leitura do ADC) determina quão alto o som precisa ser para ativar a animação. Você precisará **ajustar este valor** dependendo da sensibilidade do seu microfone e do ambiente. Comece com 2100, mas experimente valores maiores ou menores.
  * `TIMER_INTERVAL_MS`: Intervalo de tempo entre as leituras do microfone em milissegundos (padrão: `50ms`).
  * `ANIMATION_FRAMES`: Número de passos na animação para um ciclo completo (padrão: `10`).

## 🚀 Uso

Após conectar o hardware e fazer o upload do código, o Raspberry Pi Pico começará a monitorar o som.

  * Quando o nível de som estiver **abaixo** do `SOUND_THRESHOLD`, a matriz de LEDs permanecerá desligada.
  * Quando o nível de som estiver **acima** do `SOUND_THRESHOLD`, a animação de onda será ativada e exibida na matriz. A cor e a intensidade da animação variarão ligeiramente com o nível do som detectado. A animação continuará por cerca de 1 segundo após a última detecção de som alto.

## 🚶 Estrutura do Código

  * `main.c`: O arquivo principal contendo a lógica de detecção de som, controle do timer, processamento do ADC e funções de animação da matriz WS2812.
      * `put_pixel()`: Envia dados de cor para um único pixel via PIO.
      * `xy_to_index()`: Converte coordenadas X, Y da matriz para o índice linear do LED (com mapeamento serpentina).
      * `hsv_to_rgb()`: Função auxiliar para converter cores do espaço HSV para RGB.
      * `render_wave_animation()`: Calcula as cores para todos os LEDs e renderiza a animação de onda com base no nível de som.
      * `repeating_timer_callback()`: Função chamada pelo timer que realiza a leitura do ADC e chama a função de renderização da animação.
      * `main()`: Inicializa periféricos (ADC, PIO, Timer) e entra no loop principal.
  * `ws2812.pio`: Arquivo contendo o código em linguagem PIO para controlar o protocolo de comunicação do WS2812. Este arquivo é compilado pelo SDK e gera o `ws2812.pio.h`.
  * `CMakeLists.txt`: Arquivo de configuração para o sistema de build CMake.

## 💡 Notas Adicionais

  * A conversão `hsv_to_rgb` e a animação de onda são otimizações para criar um efeito visual mais interessante e responsivo do que apenas acender ou apagar LEDs.
  * O mapeamento `xy_to_index` assume um layout de matriz onde linhas alternam a direção (serpentina), o que é comum para simplificar a fiação. Ajuste esta função se sua matriz tiver um layout diferente.
  * O valor do `SOUND_THRESHOLD` é crucial para a sensibilidade. Pode ser necessário calibrá-lo experimentando diferentes valores enquanto faz barulho perto do microfone.

## Propósito

Este projeto foi desenvolvido com fins estritamente educacionais e aprendizdo durante a residência em sistemas embarcados pelo embarcatech
