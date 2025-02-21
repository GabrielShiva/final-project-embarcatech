#include <stdio.h> // inclui a biblioteca padrão para I/O 
#include <stdlib.h> // utilizar a função abs
#include "pico/stdlib.h" // inclui a biblioteca padrão do pico para gpios e temporizadores
#include "hardware/i2c.h" // inclui a biblioteca para utilizar oprotocolo i2c
#include "inc/ssd1306.h" // inclui a biblioteca com definição das funções para manipulação do display OLED
#include "inc/font.h" // inclui a biblioteca com as fontes dos caracteres para o display OLED

// definição de parametros para o protocolo i2c
#define I2C_ID i2c1
#define I2C_FREQ 400000
#define I2C_SDA 14
#define I2C_SCL 15
#define SSD_1306_ADDR 0x3C

// botões
#define BTN_A 5
#define BTN_B 6
#define BTN_SW 22

volatile bool btn_a_state = true;

volatile uint32_t last_time_btn_press = 0;
volatile uint16_t current_db_value = 10;
char db_string[10];

// inicia a estrutura do display OLED
ssd1306_t ssd;

/**
 *  0 => item de vizualização
 *  1 => item de definir nível
 *  2 => item de configuração
 */
static volatile uint current_menu_item = 0;

/**
 *  0 => página do menu principal
 *  1 => página de vizualização 
 *  2 => página de definir nível
 *  3 => página de configuração
 */
static volatile uint current_screen = 0;

const char *menu_itens[] = {
    "VIZUALIZAR", "DEF NIVEL", "CONFIGURAR"
};

// configuração do protocolo i2c
void i2c_setup() {
    // inicia o modulo i2c (i2c1) do rp2040 com uma frequencia de 400kHz
    i2c_init(I2C_ID, I2C_FREQ);

    // define o pino 14 como o barramento de dados
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    // define o pino 15 como o barramento de clock
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    // ativa os resistores internos de pull-up para os dois barramentos para evitar flutuações nos dois barramentos quando está no estado de espera (idle)
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

void btn_setup() {
    gpio_init(BTN_A);
    gpio_set_dir(BTN_A, GPIO_IN);
    gpio_pull_up(BTN_A);

    gpio_init(BTN_B);
    gpio_set_dir(BTN_B, GPIO_IN);
    gpio_pull_up(BTN_B);

    gpio_init(BTN_SW);
    gpio_set_dir(BTN_SW, GPIO_IN);
    gpio_pull_up(BTN_SW);
}

void display_init(){
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, SSD_1306_ADDR, I2C_ID); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
  
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

void inital_screen() {
    ssd1306_draw_string(&ssd, "Decimeter", 5, 3); // Desenha uma string
    // ssd1306_hline(&ssd, 0, 127, 12, true); // Desenha uma string
    ssd1306_rect(&ssd, 0, 0, 128, 14, true, false);

    // top left
    ssd1306_pixel(&ssd, 1, 1, true);
    ssd1306_pixel(&ssd, 2, 1, true);
    ssd1306_pixel(&ssd, 1, 2, true);

    // bottom left
    ssd1306_pixel(&ssd, 1, 11, true);
    ssd1306_pixel(&ssd, 1, 12, true);
    ssd1306_pixel(&ssd, 2, 12, true);

    // top right
    ssd1306_pixel(&ssd, 125, 1, true);
    ssd1306_pixel(&ssd, 126, 1, true);
    ssd1306_pixel(&ssd, 126, 2, true);
    ssd1306_send_data(&ssd);

    // bottom right
    ssd1306_pixel(&ssd, 126, 11, true);
    ssd1306_pixel(&ssd, 126, 12, true);
    ssd1306_pixel(&ssd, 125, 12, true);

    ssd1306_vline(&ssd, 78, 0, 13, true);

    ssd1306_pixel(&ssd, 76, 1, true);
    ssd1306_pixel(&ssd, 77, 1, true);
    ssd1306_pixel(&ssd, 77, 2, true);

    ssd1306_pixel(&ssd, 76, 12, true);
    ssd1306_pixel(&ssd, 77, 12, true);
    ssd1306_pixel(&ssd, 77, 11, true);

    ssd1306_pixel(&ssd, 79, 1, true);
    ssd1306_pixel(&ssd, 79, 2, true);
    ssd1306_pixel(&ssd, 80, 1, true);

    ssd1306_pixel(&ssd, 79, 12, true);
    ssd1306_pixel(&ssd, 79, 11, true);
    ssd1306_pixel(&ssd, 80, 12, true);

    // ssd1306_rect(&ssd, 14, 3, 5, 5, true, true);
    ssd1306_draw_string(&ssd, "Inicializando", 3, 25); // Desenha uma string
    
    // atualiza o display OLED
    ssd1306_send_data(&ssd);
}

void draw_left_arrow() {
    ssd1306_vline(&ssd, 11, 34, 41, true);
    ssd1306_vline(&ssd, 10, 35, 40, true);
    ssd1306_vline(&ssd, 9, 36, 39, true);
    ssd1306_vline(&ssd, 8, 37, 38, true);
}

void draw_right_arrow() {
    ssd1306_vline(&ssd, 114, 34, 41, true);
    ssd1306_vline(&ssd, 115, 35, 40, true);
    ssd1306_vline(&ssd, 116, 36, 39, true);
    ssd1306_vline(&ssd, 117, 37, 38, true);
}

void draw_plus_btn() {
    // box
    ssd1306_rect(&ssd, 105, 28, 16, 16, true, true);
    // x
    ssd1306_vline(&ssd, 112, 30, 41, false);
    ssd1306_vline(&ssd, 113, 30, 41, false);
    // y
    ssd1306_hline(&ssd, 107, 118, 35, false);
    ssd1306_hline(&ssd, 107, 118, 36, false);
}

void draw_on_led_btn() {
    // box
    ssd1306_rect(&ssd, 108, 28, 16, 16, true, btn_a_state);
}

void draw_minus_btn() {
    // box
    ssd1306_rect(&ssd, 11, 28, 16, 16, true, true);
    // x
    ssd1306_hline(&ssd, 13, 24, 35, false);
    ssd1306_hline(&ssd, 13, 24, 36, false);
}

void draw_back_arrow() {
    ssd1306_vline(&ssd, 92, 54, 61, true);
    ssd1306_vline(&ssd, 91, 55, 60, true);
    ssd1306_vline(&ssd, 90, 56, 59, true);
    ssd1306_vline(&ssd, 89, 57, 58, true);

    ssd1306_draw_string(&ssd, "vltr", 95, 54);
}

void irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

    if (current_time - last_time_btn_press > 260) {
        last_time_btn_press = current_time;

        if (gpio == BTN_A) {
            if (current_screen == 0) {
                if (current_menu_item > 0) {
                    current_menu_item = current_menu_item - 1;
                }
            } else if (current_screen == 2) {
                if (current_db_value > 0) {
                    current_db_value = current_db_value - 1;
                    printf("db: %d\n", current_db_value);
                }
            } else if (current_screen == 3) {
                btn_a_state = !btn_a_state;
            }
        } else if (gpio == BTN_B) {
            if (current_screen == 0) {
                if (current_menu_item < 2) {
                    current_menu_item = current_menu_item + 1;
                }
            } else if (current_screen == 2) {
                if (current_db_value < 20) {
                    current_db_value = current_db_value + 1;
                    printf("db: %d\n", current_db_value);
                }
            } 
        } else if (gpio == BTN_SW) {
            if (current_screen == 0) {
                if (current_menu_item == 0) {
                    current_screen = 1;
                    printf("TELA DE VIZUALIZACAO\n");
                } else if (current_menu_item == 1) {
                    current_screen = 2;
                    printf("TELA DE DEF NIVEL\n");
                } else if (current_menu_item == 2) {
                    current_screen = 3;
                    printf("TELA DE COFIGURACAO\n");
                }
            } else if (current_screen != 0) {
                current_screen = 0;
                printf("VOLTANDO PARA MENU PRINCIPAL\n");
            }
        }
    }
}

int main() {
    // chama função para comunicação serial via usb para debug
    stdio_init_all(); 

    btn_setup();

    // chama a função para configuração do protocolo i2c
    i2c_setup();

    // inicializa o display OLED
    display_init();
    inital_screen();

    sleep_ms(1500);

    snprintf(db_string, sizeof(db_string), "%ddB", current_db_value);
    ssd1306_draw_string(&ssd, db_string, 82, 3); // Desenha uma string
    ssd1306_rect(&ssd, 0, 14, 128, 50, false, true);
    // atualiza o display OLED
    
    //right arrow
    draw_right_arrow();

    ssd1306_draw_string(&ssd, menu_itens[current_menu_item], 25, 34); // Desenha uma string

    // Update the display
    ssd1306_send_data(&ssd);

    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_SW, GPIO_IRQ_EDGE_FALL, true);

    while(true) {
        if (current_screen == 0) { // menu principal
            ssd1306_rect(&ssd, 0, 14, 128, 50, false, true);

            if (current_menu_item == 0) {
                ssd1306_draw_string(&ssd, menu_itens[current_menu_item], 25, 34); // Desenha uma string
                draw_right_arrow();
                ssd1306_send_data(&ssd);
            } else if (current_menu_item == 1) {
                ssd1306_draw_string(&ssd, menu_itens[current_menu_item], 25, 34); // Desenha uma string
                draw_left_arrow();
                draw_right_arrow();
                ssd1306_send_data(&ssd);
            } else if (current_menu_item == 2) {
                ssd1306_draw_string(&ssd, menu_itens[current_menu_item], 25, 34); // Desenha uma string
                draw_left_arrow();
                ssd1306_send_data(&ssd);
            }
        } else if (current_screen == 1) { // vizualização
            ssd1306_rect(&ssd, 0, 14, 128, 50, false, true);
            draw_back_arrow();
            // progress bar
            ssd1306_rect(&ssd, 0, 20, 82, 16, true, false);
            ssd1306_rect(&ssd, 0, 20, 50, 16, true, true);
            ssd1306_draw_string(&ssd, "134dB", 84, 25); // valor medido em tempo real
            ssd1306_draw_string(&ssd, "MEDIANO", 0, 40); // valor medido em tempo real

            ssd1306_send_data(&ssd);
        } else if (current_screen == 2) { // definir nível
            ssd1306_rect(&ssd, 0, 14, 128, 50, false, true);
            draw_back_arrow();
            draw_plus_btn();
            draw_minus_btn();

            snprintf(db_string, sizeof(db_string), "%ddB", current_db_value);

            ssd1306_draw_string(&ssd, db_string, 44, 33);
            ssd1306_send_data(&ssd);
        } else if (current_screen == 3) { // configuração
            ssd1306_rect(&ssd, 0, 14, 128, 50, false, true);   
            draw_back_arrow();
            draw_on_led_btn();

            if (btn_a_state) {
                ssd1306_draw_string(&ssd, "LED LIGADO", 0, 33); // valor medido em tempo real
            } else {
                ssd1306_draw_string(&ssd, "LED DESLIGADO", 0, 33); // valor medido em tempo real
            }


            ssd1306_draw_string(&ssd, "PRESS A", 0, 55); // valor medido em tempo real
            ssd1306_send_data(&ssd);
        }

        ssd1306_rect(&ssd, 79, 1, 45, 11, false, true);
        snprintf(db_string, sizeof(db_string), "%ddB", current_db_value);
        ssd1306_draw_string(&ssd, db_string, 82, 3); // Desenha uma string

        sleep_ms(500);
    }

    return 0;
}