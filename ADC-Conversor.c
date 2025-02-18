#include <stdio.h>
#include "pico/stdlib.h"

// Pico SDK
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

// Display SSD1306
#include "inc/ssd1306.h"

// I2C defines
#define I2C_PORT i2c1
#define I2C_SCL 15
#define I2C_SDA 14

// LEDs defines
#define LED_RED 13
#define LED_BLUE 12
#define LED_GREEN 11

// PWM defines
#define PWM_FREQ 50 // Frequência do PWM em Hz
#define CLOCK_FREQ 125000000 // Frequência do clock em Hz
#define LED_WRAP 255 // Número máximo de ciclos do PWM para o LED

// Buttons defines
#define BUTTON_JS 22
#define BUTTON_A 5

// Joystick defines
#define JOYSTICK_X 26
#define JOYSTICK_Y 27
#define SQUARE_SIZE 8

// Protótipos
// Inicializações
void init_hardware(void);
void init_adc(void);
void init_leds(void);
void init_buttons(void);
void init_pwm(void);
void init_display(void);
void init_ssd1306(void);

void read_joystick(void);
void button_handler(uint gpio, uint32_t events);
void display_border(ssd1306_t *ssd, uint8_t typeBorder);
void update_square(void);
void update_leds_pwm(void);
uint8_t map_value(uint16_t value, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max);

// Variáveis globais
// LEDs
volatile bool led_state_green = false; // Estado dos LEDs
uint led_slice_blue; // Número do slice do LED Azul
uint led_slice_red; // Número do slice do LED Vermelho

// PWM
uint32_t wrap = CLOCK_FREQ / (PWM_FREQ * 64); // Número máximo de ciclos do PWM
volatile bool pwm_enabled = false; // Estado do PWM dos LEDs

ssd1306_t ssd; // Display
volatile uint8_t count_type = 0; // Tipo de borda do display

// Joystick
uint16_t adc_value_x, adc_value_y; // Valor do ADC do joystick

int main()
{
    stdio_init_all();
    init_hardware();
    printf("Iniciando...\n");

    gpio_set_irq_enabled_with_callback(BUTTON_JS, GPIO_IRQ_EDGE_FALL, true, &button_handler);
    gpio_set_irq_enabled(BUTTON_A, GPIO_IRQ_EDGE_FALL, true);

    while (true) {
        read_joystick();
        update_square();
        update_leds_pwm();
        printf("X: %d, Y: %d\n", adc_value_x, adc_value_y);
        sleep_ms(100);
    }
}

// Atualiza a intensidade do LED Azul (Y) e Vermelho (X) via PWM
void update_leds_pwm() {
    uint brightness_blue = 0;
    uint brightness_red = 0;

    // Mapeia os valores do joystick para o brilho PWM (0 a LED_WRAP)
    if (adc_value_y < 2048) {
        brightness_blue = map_value(adc_value_y, 0, 2048, LED_WRAP, 0);  // LED Azul (inverte eixo)
    } else if (adc_value_y > 2048) {
        brightness_blue = map_value(adc_value_y, 2048, 4095, 0, LED_WRAP);
    }

    if (adc_value_x < 2048) {
        brightness_red = map_value(adc_value_x, 0, 2048, LED_WRAP, 0);   // LED Vermelho (inverte eixo)
    } else if (adc_value_x > 2048) {
        brightness_red = map_value(adc_value_x, 2048, 4095, 0, LED_WRAP);
    }

    // Se o PWM estiver ativado, ajusta os LEDs
    if (pwm_enabled) {
        pwm_set_gpio_level(LED_BLUE, brightness_blue);
        pwm_set_gpio_level(LED_RED, brightness_red);
    } else {
        pwm_set_gpio_level(LED_BLUE, 0);
        pwm_set_gpio_level(LED_RED, 0);
    }
}

void button_handler(uint gpio, uint32_t events) {
    static absolute_time_t last_interrupt_time = {0};
    absolute_time_t current_time = get_absolute_time();
    int64_t time_diff = absolute_time_diff_us(last_interrupt_time, current_time);

    if (time_diff < 200000) { // 200ms debounce time
        return;
    }
    last_interrupt_time = current_time;

    if (gpio == BUTTON_JS) {
        led_state_green = !led_state_green;
        gpio_put(LED_GREEN, led_state_green);
        count_type = (count_type + 1) % 3;
    } else if (gpio == BUTTON_A) {
        pwm_enabled = !pwm_enabled;
    }
}

void display_border(ssd1306_t *ssd, uint8_t typeBorder) {
    if (typeBorder == 0) {
        ssd1306_fill(ssd, false);
        ssd1306_rect(ssd, 0, 0, WIDTH, HEIGHT, true, false);
    } else if (typeBorder == 1) {
        ssd1306_fill(ssd, false);
        for (uint8_t x = 0; x < WIDTH; x += 6) {
            ssd1306_hline(ssd, x, x + 4, 0, true);  // Linha superior
            ssd1306_hline(ssd, x, x + 4, HEIGHT - 4, true);  // Linha inferior
        }
        for (uint8_t y = 0; y < HEIGHT; y += 6) {
            ssd1306_vline(ssd, 0, y, y + 4, true);  // Linha esquerda
            ssd1306_vline(ssd, WIDTH - 1, y, y + 4, true);  // Linha direita
        }
    } else if (typeBorder == 2) {
        for (uint8_t x = 0; x < WIDTH - 4; x += 4) {
            ssd1306_pixel(ssd, x, 0, true); // Linha superior
            ssd1306_pixel(ssd, x, HEIGHT - 4, true); // Linha inferior corrigida
        }
        for (uint8_t y = 0; y < HEIGHT - 4; y += 4) {
            ssd1306_pixel(ssd, 0, y, true); // Linha esquerda
            ssd1306_pixel(ssd, WIDTH - 1, y, true); // Linha direita corrigida
        }
    }
    ssd1306_send_data(ssd);
}

// Função para ler o joystick
void read_joystick() {
    adc_select_input(1);
    adc_value_x = adc_read();
    adc_select_input(0);
    adc_value_y = adc_read();
} 

// Atualizar a posição do quadrado no display
void update_square() {
    // Mapeia os valores do joystick para a posição do quadrado
    uint8_t x = map_value(adc_value_x, 0, 4095, 0, WIDTH - SQUARE_SIZE);
    uint8_t y = map_value(adc_value_y, 0, 4095, HEIGHT - SQUARE_SIZE, 0);

    // Limpa o display
    ssd1306_fill(&ssd, false);
    display_border(&ssd, count_type);
    ssd1306_rect(&ssd, y, x, SQUARE_SIZE, SQUARE_SIZE, true, true);
    ssd1306_send_data(&ssd);
}

// Função para mapear valores do ADC (0-4095) para posição do quadrado (0-120 para X e 0-56 para Y)
uint8_t map_value(uint16_t value, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max) {
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Inicializações
void init_hardware(void) {
    init_adc();
    init_leds();
    init_buttons();
    init_pwm();
    init_display();
    init_ssd1306();
    sleep_ms(500);
}

void init_adc(void) {
    // Inicializa o ADC
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_gpio_init(JOYSTICK_Y);
    printf("ADC inicializado\n");
    sleep_ms(500);
}

void init_leds(void) {
    // Inicializa o LED Verde
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    printf("LED Verde inicializado\n");
    sleep_ms(500);
}

void init_buttons(void) {
    // Inicializa os botões
    gpio_init(BUTTON_JS);
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_JS, GPIO_IN);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_JS);
    gpio_pull_up(BUTTON_A);
    printf("Botões inicializados\n");
    sleep_ms(500);
}

void init_pwm(void) {
    // Inicializa o PWM para o LED Azul e Vermelho
    gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);
    led_slice_blue = pwm_gpio_to_slice_num(LED_BLUE);
    led_slice_red = pwm_gpio_to_slice_num(LED_RED);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, LED_WRAP); // PWM com resolução de 8 bits

    pwm_init(led_slice_blue, &config, true);
    pwm_init(led_slice_red, &config, true);
    printf("PWM inicializado\n");
    sleep_ms(500);
}

void init_display(void) {
    // Inicializa o I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    printf("I2C inicializado\n");
    sleep_ms(500);
}

void init_ssd1306(void) {
    // Inicializa o display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, 0x3C, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
    sleep_ms(500);
}