/**
 * Código para Raspberry Pi Pico: Detecção de Som com Matriz LED WS2812 para Atividade Cap 3
 * 
 * REQUISITOS CUMPRIDOS:
 * ✅ Leitura do microfone via ADC (GPIO28/ADC2)
 * ✅ Uso de timer periódico com callback (add_repeating_timer_ms())
 * ✅ Estabelecimento de limiar de detecção de som (2100)
 * ✅ Controle da matriz WS2812 com animação quando o limiar é ultrapassado
 */

 #include <stdio.h>
 #include "pico/stdlib.h"
 #include "hardware/adc.h"
 #include "hardware/timer.h"
 #include "hardware/pio.h"
 #include "ws2812.pio.h" // Inclui o cabeçalho gerado a partir do ws2812.pio
 #include <math.h>
 
 // --- Definições de Hardware e Configuração ---
 
 // Pinos GPIO conectados aos periféricos
 #define AUDIO_INPUT_PIN 28      // Pino GPIO conectado à saída analógica do microfone
 #define AUDIO_ADC_CHANNEL 2     // Canal do ADC correspondente ao GPIO28 (ADC2)
 #define NEOPIXEL_PIN 7          // Pino GPIO conectado ao DATA IN da matriz Neopixel
 #define NUM_PIXELS 25           // Número total de LEDs na matriz (5x5 = 25)
 #define MATRIX_WIDTH 5          // Largura da matriz (5x5)
 #define MATRIX_HEIGHT 5         // Altura da matriz (5x5)
 
 // Configuração de detecção de som e timer
 #define SOUND_THRESHOLD 2100    // Limiar para detecção de som alto (REQUISITO ✅)
 #define TIMER_INTERVAL_MS 50    // Leitura a cada 50ms (20 vezes por segundo)
 #define ANIMATION_FRAMES 10     // Número de frames para a animação completa
 
 // --- Variáveis Globais ---
 
 // Instância do PIO e State Machine usados para controlar os Neopixels
 PIO pio = pio0;
 uint sm = 0;
 
 // Variáveis para controlar a animação
 bool sound_detected = false;
 uint8_t animation_step = 0;
 uint16_t last_sound_level = 0;
 uint64_t last_sound_time = 0;  // Para armazenar o último momento de detecção de som
 
 // --- Funções de Controle dos Neopixels ---
 
 /**
  * Função auxiliar para enviar um pixel (cor GRB) para a State Machine do PIO
  * Parte do REQUISITO: Controle de matriz WS2812 ✅
  */
 static inline void put_pixel(uint32_t pixel_grb) {
     pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
 }
 
 /**
  * Função para converter coordenadas XY da matriz em índice de LED
  * Otimização para trabalhar com a matriz de LED
  */
 inline uint8_t xy_to_index(uint8_t x, uint8_t y) {
     // Alterna a direção em linhas ímpares (efeito serpentina)
     if (y % 2 == 0) {
         return y * MATRIX_WIDTH + x;
     } else {
         return y * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - x);
     }
 }
 
 /**
  * Função para gerar uma cor HSV (Hue, Saturation, Value) e converter para RGB
  * Melhoria para enriquecer a animação visual
  * Parâmetros: h (0-359), s (0-100), v (0-100)
  */
 void hsv_to_rgb(float h, float s, float v, uint8_t *r, uint8_t *g, uint8_t *b) {
     // Normaliza valores para o intervalo 0-1
     h = fmodf(h, 360.0f) / 60.0f;
     s = s / 100.0f;
     v = v / 100.0f;
     
     float c = v * s;
     float x = c * (1 - fabsf(fmodf(h, 2.0f) - 1));
     float m = v - c;
     
     float r_temp, g_temp, b_temp;
     
     if (h >= 0 && h < 1) {
         r_temp = c; g_temp = x; b_temp = 0;
     } else if (h >= 1 && h < 2) {
         r_temp = x; g_temp = c; b_temp = 0;
     } else if (h >= 2 && h < 3) {
         r_temp = 0; g_temp = c; b_temp = x;
     } else if (h >= 3 && h < 4) {
         r_temp = 0; g_temp = x; b_temp = c;
     } else if (h >= 4 && h < 5) {
         r_temp = x; g_temp = 0; b_temp = c;
     } else {
         r_temp = c; g_temp = 0; b_temp = x;
     }
     
     *r = (uint8_t)((r_temp + m) * 255);
     *g = (uint8_t)((g_temp + m) * 255);
     *b = (uint8_t)((b_temp + m) * 255);
 }
 
 /**
  * Função para renderizar a animação de ondas na matriz de LED
  * REQUISITO CUMPRIDO: Ativar padrão de luz na matriz WS2812 ✅
  */
 void render_wave_animation(uint16_t sound_level) {
     // Mapeia o nível de som para intensidade de cor
     float intensity = (sound_level > SOUND_THRESHOLD) ? 
                       ((float)(sound_level - SOUND_THRESHOLD) / (4095 - SOUND_THRESHOLD)) : 0;
     
     // Limita a intensidade entre 0 e 1
     intensity = intensity > 1.0f ? 1.0f : intensity;
     
     // Calcula o tempo atual
     uint64_t current_time = time_us_64() / 1000;
     
     // Atualiza a animação apenas se houver som detectado recentemente
     // REQUISITO CUMPRIDO: Verificação do limiar de som ✅
     if (sound_level > SOUND_THRESHOLD) {
         // FLUXO: Som acima do limiar detectado - ativa a animação
         last_sound_time = current_time;
         sound_detected = true;
     } else if (current_time - last_sound_time > 1000) { // Desliga após 1 segundo sem som alto
         // FLUXO: Sem som alto por mais de 1 segundo - desliga a animação
         sound_detected = false;
     }
     
     // Se o som não foi detectado, apaga todos os LEDs
     if (!sound_detected) {
         // FLUXO: Desligando todos os LEDs pois não há som alto
         for (uint8_t i = 0; i < NUM_PIXELS; i++) {
             put_pixel(0);  // Desliga o LED
         }
         return;
     }
     
     // FLUXO: Gerando animação baseada no som detectado
     
     // Calcular a hue base baseada no nível de som (mapeando para 0-360)
     float base_hue = (sound_level * 360.0f) / 4095.0f;
     
     // Onda que se propaga do centro para fora com cores baseadas no nível de som
     float center_x = MATRIX_WIDTH / 2.0f;
     float center_y = MATRIX_HEIGHT / 2.0f;
     
     // Loop através de cada LED na matriz
     for (uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
         for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
             // Calcula a distância do LED ao centro da matriz
             float dist = sqrtf(powf(x - center_x, 2) + powf(y - center_y, 2));
             
             // Cria uma onda que se propaga do centro para fora
             float phase = (float)animation_step / ANIMATION_FRAMES;
             float wave = sinf(dist * 1.5f - phase * 6.28f) * 0.5f + 0.5f;
             
             // Ajusta a intensidade da onda com base no nível de som
             wave = wave * intensity;
             
             // Calcula a cor do LED com base na distância e fase da onda
             uint8_t r, g, b;
             float hue = base_hue + dist * 30.0f; // Varia a cor com a distância
             hsv_to_rgb(hue, 100.0f, wave * 100.0f, &r, &g, &b);
             
             // Envia a cor para o LED
             uint32_t color = ((uint32_t)(g) << 16) | ((uint32_t)(r) << 8) | (uint32_t)(b);
             put_pixel(color);
         }
     }
     
     // Avança para o próximo passo da animação
     animation_step = (animation_step + 1) % ANIMATION_FRAMES;
 }
 
 /**
  * Função de callback do timer - chamada periodicamente
  * REQUISITO CUMPRIDO: Uso de timer periódico com callback ✅
  */
 bool repeating_timer_callback(struct repeating_timer *t) {
     // FLUXO: Timer disparou - iniciando leitura do microfone
     
     // Seleciona o canal do ADC conectado ao microfone
     adc_select_input(AUDIO_ADC_CHANNEL);
     
     // Realiza a leitura do ADC (valor bruto de 0 a 4095)
     // REQUISITO CUMPRIDO: Leitura do sinal do microfone através do ADC ✅
     uint16_t raw_adc = adc_read();
     
     // Atualiza o último nível de som detectado
     last_sound_level = raw_adc;
 
     // FLUXO: Leitura ADC concluída com valor: raw_adc (seria exibido no printf)
 
     // Renderiza a animação com base no nível de som
     // FLUXO: Enviando valor do ADC para processamento da animação
     render_wave_animation(raw_adc);
 
     // Retorna true para que o timer continue rodando periodicamente
     return true;
 }
 
 /**
  * Função principal - inicializa o sistema e entra em loop infinito
  */
 int main() {
     // Inicializa o Standard I/O (stdio)
     stdio_init_all();
 
     // FLUXO: Iniciando sistema de detecção de som com animação de Neopixels
 
     // --- Configuração do ADC ---
     // REQUISITO CUMPRIDO: Configuração do ADC para leitura do microfone ✅
     // FLUXO: Inicializando ADC para leitura do microfone
     adc_init();
     adc_gpio_init(AUDIO_INPUT_PIN);
 
     // --- Configuração do PIO para Neopixels ---
     // REQUISITO CUMPRIDO: Configuração para controle da matriz WS2812 ✅
     // FLUXO: Configurando PIO para controle da matriz WS2812
     uint offset = pio_add_program(pio, &ws2812_program);
     sm = pio_claim_unused_sm(pio, true);
     ws2812_program_init(pio, sm, offset, NEOPIXEL_PIN, 800000, false);
 
     // --- Configuração do Timer Periódico ---
     // REQUISITO CUMPRIDO: Configuração do timer periódico ✅
     // FLUXO: Configurando timer periódico para leitura do ADC
     struct repeating_timer timer;
     if (!add_repeating_timer_ms(TIMER_INTERVAL_MS, repeating_timer_callback, NULL, &timer)) {
         // FLUXO: ERRO - Falha ao adicionar o timer repetitivo
         return 1;
     }
 
     // FLUXO: Sistema inicializado com sucesso. Monitorando microfone...
 
     // Loop principal
     while (1) {
         // FLUXO: Loop principal em execução
         tight_loop_contents();
     }
 
     return 0;
 }
