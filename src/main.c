#include <stdio.h>
#include <stdlib.h> 
#include <math.h>

#include "pico/stdlib.h"

#include "hardware/adc.h"

#include "inc/display/display.h"

// Definição de parâmetros para o protocolo I2C
#define I2C_ID i2c1
#define I2C_FREQ 400000
#define I2C_SDA 14
#define I2C_SCL 15
#define SSD_1306_ADDR 0x3C

// Define os pinos dos botões
#define BTN_A 5
#define BTN_B 6
#define BTN_SW 22

// Define os pinos do microfone
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)

// Define constantes para representar as telas
#define PAGE_MENU 0
#define PAGE_MEASUREMENT 1
#define PAGE_DEFINE_LEVEL 2
#define PAGE_CONFIGURATION 3

// Define os valores máximo e mínimo para configuração
#define DB_MIN 0
#define DB_MAX 150

// Define os valores para a progress-bar
#define MAX_DB 150                
#define PROGRESS_BAR_X 0          
#define PROGRESS_BAR_Y 20       
#define PROGRESS_BAR_WIDTH 82 
#define PROGRESS_BAR_HEIGHT 16

// Define e inicializa variável que armazena o item atual do menu principal
//  0 => item de vizualização
//  1 => item de definir nível
//  2 => item de configuração
static volatile uint current_menu_item = 0;

// Define e inicializa variável que armazena a página atual exibida na GUI
//  0 => página do menu principal
//  1 => página de vizualização 
//  2 => página de definir nível
//  3 => página de configuração
static volatile uint current_screen = 0;

// Define variável para debounce do botão
volatile uint32_t last_time_btn_press = 0;

// Define e inicializa a variável que armazena o valor limite em dB definido pelo usuáriu
volatile uint db_value_boundary = 60;

// Define e inicializa, em dB, o valor medido pelo microfone MAX4466
volatile uint db_value = 0;

// Define variável que armazena o texto que será exibido na página de visualização
char db_string[10];
char db_measured_string[10];

volatile uint peak_to_peak = 0;
volatile uint signal_max = 0;
volatile uint signal_min = 4095;

// Define e armazena o estado do botão A
volatile bool btn_a_state = true;

// Define os itens do menu principal
const char *menu_itens[] = {
    "VIZUALIZAR", "DEF NIVEL", "CONFIGURAR"
};

const int sample_window = 50;  // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

// Configura e inicializa os botões
void btn_setup(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
}

// Configura os periféricos: botões, I2C e display
void peripheral_setup() {
    // Inicializa e configura os botões: A, B e SW
    btn_setup(BTN_A);
    btn_setup(BTN_B);
    btn_setup(BTN_SW);

    // Chama a função para configuração do protocolo I2C (utilizado na comunicação com o display)
    i2c_setup(I2C_ID, I2C_FREQ, I2C_SDA, I2C_SCL);
    
    // Inicializa o display
    display_setup(SSD_1306_ADDR, I2C_ID);
}

// Atualiza a barra de progresso
void update_progress_bar(uint db_value) {
    // Limita o valor mínimo
    if (db_value < 0) {
        db_value = 0;
    }

     // Limita o valor máximo
    if (db_value > MAX_DB) { 
        db_value = MAX_DB;
    }

    // Determina o comprimento da barra que estará preenchida
    uint filled_width = (db_value * PROGRESS_BAR_WIDTH) / MAX_DB;

    // Desenha o contorno da barra de progresso na tela de visualização
    ssd1306_rect(&ssd, PROGRESS_BAR_X, PROGRESS_BAR_Y, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, true, false);

    // Draw the filled part based on current intensity
    ssd1306_rect(&ssd, PROGRESS_BAR_X, PROGRESS_BAR_Y, filled_width, PROGRESS_BAR_HEIGHT, true, true);
}

// Define função que exibe a p´ágina selecionada no GUI
void call_page(uint page_selected) {
    if (page_selected == PAGE_MENU) {
        if (current_menu_item == 0) {
            ssd1306_draw_string(&ssd, menu_itens[current_menu_item], 25, 34); // Desenha uma string
            display_draw_right_arrow();
            ssd1306_send_data(&ssd);
        } else if (current_menu_item == 1) {
            ssd1306_draw_string(&ssd, menu_itens[current_menu_item], 25, 34); // Desenha uma string
            display_draw_left_arrow();
            display_draw_right_arrow();
            ssd1306_send_data(&ssd);
        } else if (current_menu_item == 2) {
            ssd1306_draw_string(&ssd, menu_itens[current_menu_item], 25, 34); // Desenha uma string
            display_draw_left_arrow();
            ssd1306_send_data(&ssd);
        }
    } else if (page_selected == PAGE_DEFINE_LEVEL) {
        display_draw_back_arrow();
        display_draw_plus_btn();
        display_draw_minus_btn();
    
        snprintf(db_string, sizeof(db_string), "%ddB", db_value_boundary);
    
        ssd1306_draw_string(&ssd, db_string, 44, 33);
        ssd1306_send_data(&ssd);
    } else if (page_selected == PAGE_MEASUREMENT) {
        // Adiciona o botão de voltar na GUI
        display_draw_back_arrow();
        // Atualiza a barra de progresso
        update_progress_bar(db_value);

        // Cria a string que é exibida ao lado da barra de progresso. Exibe o valor medido em tempo real (dB)
        snprintf(db_measured_string, sizeof(db_measured_string), "%ddB", db_value);
        ssd1306_draw_string(&ssd, db_measured_string, 84, 25); 
        
        ssd1306_draw_string(&ssd, "MEDIANO", 0, 40);
    
        ssd1306_send_data(&ssd);
    } else if (page_selected == PAGE_CONFIGURATION) {
        display_draw_back_arrow();
        display_draw_led_on_btn(btn_a_state);

        if (btn_a_state) {
            ssd1306_draw_string(&ssd, "LED LIGADO", 0, 33); // valor medido em tempo real
        } else {
            ssd1306_draw_string(&ssd, "LED DESLIGADO", 0, 33); // valor medido em tempo real
        }

        ssd1306_draw_string(&ssd, "PRESS A", 0, 55); // valor medido em tempo real
        ssd1306_send_data(&ssd);
    }
}

// Realiza a medição do microfone
uint mic_measurement() {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    peak_to_peak = 0;
    signal_max = 0;
    signal_min = 4095;

    while(to_ms_since_boot(get_absolute_time()) - current_time < sample_window) {
        sample = adc_read();

        if (sample < 4095) {
            if (sample > signal_max) {
                signal_max = sample;  
            } else if (sample < signal_min) {
                signal_min = sample; 
            }
        }
    }

    return (signal_max - signal_min);
}

// Converte o valor pico a pico para dB
uint convert_to_db(uint peak_to_peak) {
    return ((uint) round(20.0 * log10((double) peak_to_peak)));   
}
  
void adc_setup() {
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);
}

// Função que trata das interrupções geradas pelos botões
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
                if (db_value_boundary > DB_MIN) {
                    db_value_boundary = db_value_boundary - 1;
                    printf("db: %d\n", db_value_boundary);
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
                if (db_value_boundary < DB_MAX) {
                    db_value_boundary = db_value_boundary + 1;
                    printf("db: %d\n", db_value_boundary);
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
    // Chama função para comunicação serial via usb para depuração
    stdio_init_all(); 

    // Inicializa os periféricos: botões A, B e SW; display ssd1306
    peripheral_setup();

    // Desenha a GUI no display
    display_draw_main_layout();

    // Insere o texto de inicialização na GUI
    ssd1306_rect(&ssd, 0, 14, 128, 50, false, true);
    ssd1306_draw_string(&ssd, "Inicializando", 5, 25); 
    ssd1306_send_data(&ssd);
    sleep_ms(1500);

    // Insere o texto de configuração do ADC na GUI
    ssd1306_rect(&ssd, 0, 14, 128, 50, false, true);
    ssd1306_draw_string(&ssd, "Config ADC", 5, 25); 
    ssd1306_send_data(&ssd);

    // Inicializa o ADC para o microfone
    adc_setup();
    sleep_ms(1500);
    
    snprintf(db_string, sizeof(db_string), "%udB", db_value_boundary);

    ssd1306_draw_string(&ssd, db_string, 83, 3); 
    ssd1306_send_data(&ssd);

    // Desenha a GUI do menu principal
    display_clean_main_area();    
    display_draw_right_arrow();
    ssd1306_draw_string(&ssd, menu_itens[current_menu_item], 25, 34);
    ssd1306_send_data(&ssd);

    // Configura e habilita interrupções para os botões
    gpio_set_irq_enabled_with_callback(BTN_A, GPIO_IRQ_EDGE_FALL, true, &irq_handler);
    gpio_set_irq_enabled(BTN_B, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTN_SW, GPIO_IRQ_EDGE_FALL, true);

    while(true) {
        // Limpa o buffer da área principal
        display_clean_main_area();

        // Exibe a página atual na GUI
        call_page(current_screen);

        // Chama a função que atualiza o valor, em dB, que é exibido no cabeçalho
        ssd1306_rect(&ssd, 79, 1, 45, 11, false, true);
        snprintf(db_string, sizeof(db_string), "%udB", db_value_boundary);
        ssd1306_draw_string(&ssd, db_string, 83, 3);

        // Realiza a medição do microfone 
        peak_to_peak = mic_measurement();
        db_value = convert_to_db(peak_to_peak);

        printf("Sound level: %d dB\n", db_value);
    }

    return 0;
}