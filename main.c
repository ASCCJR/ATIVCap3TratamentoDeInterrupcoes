#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/pio.h"
#include "ws2812.pio.h" // Inclui o cabeçalho gerado a partir do ws2812.pio

// --- Definições de Hardware e Configuração ---

// Pinos GPIO conectados aos periféricos
#define AUDIO_INPUT_PIN 28      // Pino GPIO conectado à saída analógica do microfone
#define AUDIO_ADC_CHANNEL 2     // Canal do ADC correspondente ao GPIO28 (ADC2)
#define NEOPIXEL_PIN 7          // Pino GPIO conectado ao DATA IN da matriz Neopixel
#define NUM_PIXELS 25           // Número total de LEDs na matriz (5x5 = 25)

// Configuração de detecção de som e timer
// Limiar do ADC para detecção de som (ajuste conforme a leitura do seu microfone)
// O ADC é de 12 bits, valores de 0 a 4095. Um valor comum de repouso é ~2048.
// 800 é apenas um valor inicial baixo, provavelmente precisará ser ajustado para um valor > 2048
#define SOUND_THRESHOLD 2100     // Exemplo: Ajuste este limiar para um valor acima do nível de ruído/repouso

// Intervalo do timer em milissegundos para ler o microfone periodicamente
#define TIMER_INTERVAL_MS 50    // Leitura a cada 50ms (20 vezes por segundo)

// --- Variáveis Globais ---

// Instância do PIO e State Machine usados para controlar os Neopixels
PIO pio = pio0;
uint sm = 0;

// --- Funções de Controle dos Neopixels ---

// Função auxiliar para enviar um pixel (cor GRB) para a State Machine do PIO
// A cor é no formato GGRRBB, mas o programa PIO espera GRB e faz o shift de 8 bits
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// Função para definir a cor de todos os LEDs na matriz
void fill_matrix(uint8_t r, uint8_t g, uint8_t b) {
    // Monta a cor no formato GRB (os Neopixels esperam essa ordem)
    uint32_t pixel_color = ((uint32_t)(g) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(b) << 8);
    // Envia a mesma cor para todos os pixels na matriz
    for (uint16_t i = 0; i < NUM_PIXELS; i++) {
        put_pixel(pixel_color);
    }
}

// --- Função de Callback do Timer ---

// Esta função é chamada periodicamente pelo timer de hardware
bool repeating_timer_callback(struct repeating_timer *t) {
    // Seleciona o canal do ADC conectado ao microfone
    adc_select_input(AUDIO_ADC_CHANNEL);
    // Realiza a leitura do ADC (valor bruto de 0 a 4095)
    uint16_t raw_adc = adc_read();

    // Imprime o valor lido no monitor serial para depuração
    printf("ADC Value: %d\n", raw_adc);

    // Verifica se o valor lido excede o limiar de detecção de som
    if (raw_adc > SOUND_THRESHOLD) {
        // Se exceder, ativa o padrão de luz (todos brancos)
        fill_matrix(255, 255, 255); // Cor Branca (R=255, G=255, B=255)
    } else {
        // Se não exceder, desliga os LEDs (cor preta)
        fill_matrix(0, 0, 0);       // Cor Preta (R=0, G=0, B=0) - desliga os LEDs
    }

    // Retorna true para que o timer continue rodando periodicamente
    return true;
}

// --- Função Principal ---

int main() {
    // Inicializa o Standard I/O (stdio) para usar printf
    // A configuração no CMakeLists.txt determina se será via USB ou UART
    stdio_init_all();

    printf("Iniciando deteccao de som com Neopixels...\n");

    // --- Configuração do ADC ---
    // Inicializa o periférico ADC
    adc_init();
    // Habilita o pino GPIO específico para ser usado como entrada analógica do ADC
    adc_gpio_init(AUDIO_INPUT_PIN);

    // --- Configuração do PIO para Neopixels ---
    // Adiciona o programa PIO compilado (do ws2812.pio.h) à instância do PIO
    uint offset = pio_add_program(pio, &ws2812_program);
    // Reivindica uma State Machine (máquina de estado) não usada no PIO
    sm = pio_claim_unused_sm(pio, true);
    // Configura a State Machine com o programa, pino de dados e frequência de bits do WS2812 (800kHz)
    ws2812_program_init(pio, sm, offset, NEOPIXEL_PIN, 800000, false);

    // --- Configuração do Timer Periódico ---
    // Estrutura para manter as informações do timer
    struct repeating_timer timer;
    // Adiciona um timer que chamará a função repeating_timer_callback a cada TIMER_INTERVAL_MS
    if (!add_repeating_timer_ms(TIMER_INTERVAL_MS, repeating_timer_callback, NULL, &timer)) {
        printf("Erro: Falha ao adicionar o timer repetitivo!\n");
        // Pode adicionar um loop while(1) aqui para parar o programa em caso de erro
        return 1; // Indica um erro
    }

    printf("Sistema inicializado. Monitorando microfone...\n");

    // O programa principal entra em um loop infinito.
    // A lógica de leitura do microfone e controle dos LEDs é feita na função de callback do timer.
    while (1) {
        // tight_loop_contents() é uma função otimizada que pode ser usada em loops vazios
        // para reduzir o consumo de energia (embora com printf no timer, o CPU estará ativo).
        tight_loop_contents();
    }

    // Este ponto não deve ser alcançado em um loop infinito
    return 0;
}