#include <stdio.h> // inclui a biblioteca padrão para I/O 
#include <stdlib.h> // utilizar a função abs
#include "pico/stdlib.h" // inclui a biblioteca padrão do pico para gpios e temporizadores
#include "hardware/i2c.h" // inclui a biblioteca para utilizar oprotocolo i2c
#include "inc/ssd1306/ssd1306.h" // inclui a biblioteca com definição das funções para manipulação do display OLED
#include "inc/ssd1306/font.h" // inclui a biblioteca com as fontes dos caracteres para o display OLED

// Inicia a estrutura do display OLED
ssd1306_t ssd;

// Configuração do protocolo i2c
void i2c_setup(i2c_inst_t *id, uint freq, uint sda_pin, uint scl_pin) {
    // inicia o modulo i2c (i2c1) do rp2040 com uma frequencia de 400kHz
    i2c_init(id, freq);

    // define o pino 14 como o barramento de dados
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    // define o pino 15 como o barramento de clock
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);

    // ativa os resistores internos de pull-up para os dois barramentos para evitar flutuações nos dois barramentos quando está no estado de espera (idle)
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
}

// Configura o display
void display_setup(uint8_t address, i2c_inst_t *i2c_id){
    // Inicializa e configura o display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, address, i2c_id); 
    ssd1306_config(&ssd); 
    ssd1306_send_data(&ssd); 

    // Limpa o buffer e atualiza o display
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// Desenha o layout principal da GUI
void display_draw_main_layout() {
    // Define o cabeçalho do layout
    ssd1306_draw_string(&ssd, "Decimeter", 5, 3); 
    ssd1306_rect(&ssd, 0, 0, 128, 14, true, false);

    // Arredodamento - topo esquerda
    ssd1306_pixel(&ssd, 1, 1, true);
    ssd1306_pixel(&ssd, 2, 1, true);
    ssd1306_pixel(&ssd, 1, 2, true);

    // Arredodamento - base esquerda
    ssd1306_pixel(&ssd, 1, 11, true);
    ssd1306_pixel(&ssd, 1, 12, true);
    ssd1306_pixel(&ssd, 2, 12, true);

    // Arredodamento - topo direito
    ssd1306_pixel(&ssd, 125, 1, true);
    ssd1306_pixel(&ssd, 126, 1, true);
    ssd1306_pixel(&ssd, 126, 2, true);

    // Arredodamento - base direita
    ssd1306_pixel(&ssd, 126, 11, true);
    ssd1306_pixel(&ssd, 126, 12, true);
    ssd1306_pixel(&ssd, 125, 12, true);

    // Linha divisório do cabeçalho
    ssd1306_vline(&ssd, 78, 0, 13, true);

    // Arredodamento - topo esquerda
    ssd1306_pixel(&ssd, 76, 1, true);
    ssd1306_pixel(&ssd, 77, 1, true);
    ssd1306_pixel(&ssd, 77, 2, true);

    // Arredodamento - base esquerda
    ssd1306_pixel(&ssd, 76, 12, true);
    ssd1306_pixel(&ssd, 77, 12, true);
    ssd1306_pixel(&ssd, 77, 11, true);

    // Arredodamento - topo direito
    ssd1306_pixel(&ssd, 79, 1, true);
    ssd1306_pixel(&ssd, 79, 2, true);
    ssd1306_pixel(&ssd, 80, 1, true);

    // Arredodamento - base direita
    ssd1306_pixel(&ssd, 79, 12, true);
    ssd1306_pixel(&ssd, 79, 11, true);
    ssd1306_pixel(&ssd, 80, 12, true);
    
    // atualiza o display OLED
    ssd1306_send_data(&ssd);
}

// Limpa a seção principal da GUI
void display_clean_main_area() {
    ssd1306_rect(&ssd, 0, 14, 128, 50, false, true);
}

// Insere a seta para a esquerda na GUI
void display_draw_left_arrow() {
    ssd1306_vline(&ssd, 11, 34, 41, true);
    ssd1306_vline(&ssd, 10, 35, 40, true);
    ssd1306_vline(&ssd, 9, 36, 39, true);
    ssd1306_vline(&ssd, 8, 37, 38, true);
}

// Insere a seta para a direita na GUI
void display_draw_right_arrow() {
    ssd1306_vline(&ssd, 114, 34, 41, true);
    ssd1306_vline(&ssd, 115, 35, 40, true);
    ssd1306_vline(&ssd, 116, 36, 39, true);
    ssd1306_vline(&ssd, 117, 37, 38, true);
}

// Insere o botão de incremento na GUI
void display_draw_plus_btn() {
    // Desenha o botão
    ssd1306_rect(&ssd, 105, 28, 16, 16, true, true);

   // Desenha o sinal de mais
   // Eixo x
    ssd1306_vline(&ssd, 112, 30, 41, false);
    ssd1306_vline(&ssd, 113, 30, 41, false);
   // Eixo y
    ssd1306_hline(&ssd, 107, 118, 35, false);
    ssd1306_hline(&ssd, 107, 118, 36, false);
}

// Insere o botão de decremento na GUI
void display_draw_minus_btn() {
    // Desenha botão
    ssd1306_rect(&ssd, 11, 28, 16, 16, true, true);
    // Desenha o sinal de menos
    ssd1306_hline(&ssd, 13, 24, 35, false);
    ssd1306_hline(&ssd, 13, 24, 36, false);
}

// Insere a caixa que representa o estado do LED na GUI
void display_draw_led_on_btn(bool btn_state) {
    // Desenha caixa de estado
    ssd1306_rect(&ssd, 108, 28, 16, 16, true, btn_state);
}

// Desenha o botão de retorno na GUI
void display_draw_back_arrow() {
    // Desenha o sinal de retorno
    ssd1306_vline(&ssd, 92, 54, 61, true);
    ssd1306_vline(&ssd, 91, 55, 60, true);
    ssd1306_vline(&ssd, 90, 56, 59, true);
    ssd1306_vline(&ssd, 89, 57, 58, true);

    ssd1306_draw_string(&ssd, "vltr", 95, 54);
}

