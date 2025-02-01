#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "numeros.h"

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define tempo 400

static volatile int contador = 0;

const uint led_pin_red   = 13; //Led rgb vermelho
const uint button_A = 5; // Botão A = 5
const uint button_B = 6; // Botão B = 6
// Variável global para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t led_r = 0; // Intensidade do vermelho
uint8_t led_g = 50; // Intensidade do verde
uint8_t led_b = 0; // Intensidade do azul

// Buffer para armazenar quais LEDs estão ligados matriz 5x5
bool led_buffer[NUM_PIXELS] = {
    1, 0, 0, 0, 1, 
    0, 0, 0, 0, 0, 
    0, 0, 1, 0, 0, 
    0, 0, 0, 0, 0, 
    1, 0, 0, 0, 1
};

bool* numeros[10] = {numeroZero, numeroUm, numeroDois, numeroTres, numeroQuatro, numeroCinco, numeroSeis, numeroSete, numeroOito, numeroNove};

void ligarMatrizLeds();
void blinkarLedVermelho();
bool debouncing();
//uma única rotina de interrupção para simplificar a lógica do software
static void gpio_irq_handler(uint gpio, uint32_t events);

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}



void set_one_led(uint8_t r, uint8_t g, uint8_t b, int numExibir)
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);
    
    for (int j = 0; j < 25; j++) {
    led_buffer[j] = numeros[numExibir][j];
    }

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {   
             
        if (led_buffer[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}

int main()
{   
    stdio_init_all();
    int contador = 0;
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    gpio_init(led_pin_red);              // Inicializa o pino do LED
    gpio_set_dir(led_pin_red, GPIO_OUT); // Configura o pino como saída
    
    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(button_A);

    gpio_init(button_B);
    gpio_set_dir(button_B, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(button_B);

    gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (1)
    {   
        ligarMatrizLeds();
        blinkarLedVermelho();
        sleep_ms(100); // Adicionei um delay para evitar que o loop rode muito rápido
    }

    return 0;
}

void ligarMatrizLeds(){
    //implementar a logica de exibição do numero na matriz, utilize o contador "contador"
    set_one_led(led_r, led_g, led_b, contador);
}

void blinkarLedVermelho(){
    static bool led_state = false; // Estado do LED (ligado/desligado)
    static absolute_time_t last_time = {0}; // Última vez que o LED mudou de estado
   
    absolute_time_t current_time = get_absolute_time();

    // 100 ms (0.1 segundos) desde a última vez que o LED mudou de estado
    if (absolute_time_diff_us(last_time, current_time) >= 100000) {
       
        if (gpio_get(led_pin_red)){
            gpio_put(led_pin_red, 0); // Desliga o LED vermelho
        } else {
            gpio_put(led_pin_red, 1); // Liga o LED vermelho
        }
        // Atualize o tempo da última mudança de estado
        last_time = current_time;
    }

}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    
    static uint32_t last_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_time < 200) {
        return;
    }
    last_time = current_time;
    
    if(gpio == button_B && contador) // se maior q zero
        contador--;
    printf("Contador: %d\n", contador);

    if(gpio == button_A && contador < 9) // se maior q zero
        contador++;
    printf("Contador: %d\n", contador);
}