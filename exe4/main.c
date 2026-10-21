#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/adc.h"

const int PIN_LED_B = 4;
const int PIN_ADC = 28;      
const int ADC_CHANNEL = 2;

const float conversion_factor = 3.3f / (1 << 12);

const float VOLT_ZONE1 = 1.0f;
const float VOLT_ZONE2 = 2.0f;
const int PERIOD_ZONE1_MS = 300;
const int PERIOD_ZONE2_MS = 500;

/**
 * 0..1.0V: Desligado
 * 1..2.0V: 300 ms
 * 2..3.3V: 500 ms
 */

// Zona 0: desligado, zona 1: 300 ms, zona 2: 500 ms
int get_zone(float volts) {
    if (volts < VOLT_ZONE1) {
        return 0;
    }
    if (volts < VOLT_ZONE2) {
        return 1;
    }
    return 2;
}

// Só inverte o LED: o callback fica curto
bool timer_callback(repeating_timer_t *rt) {
    gpio_put(PIN_LED_B, !gpio_get_out_level(PIN_LED_B));
    return true;
}

int main() {
    stdio_init_all();

    gpio_init(PIN_LED_B);
    gpio_set_dir(PIN_LED_B, GPIO_OUT);
    gpio_put(PIN_LED_B, 0);

    adc_init();
    adc_gpio_init(PIN_ADC);
    adc_select_input(ADC_CHANNEL);

    repeating_timer_t timer;
    bool timer_active = false;
    int last_zone = -1;   // força aplicar a zona na primeira leitura

    while (1) {
        int zone = get_zone(adc_read() * conversion_factor);

        // só mexe no timer quando a zona muda
        if (zone != last_zone) {
            last_zone = zone;

            if (timer_active) {
                cancel_repeating_timer(&timer);
                timer_active = false;
            }
            gpio_put(PIN_LED_B, 0);

            if (zone != 0) {
                int period = (zone == 1) ? PERIOD_ZONE1_MS : PERIOD_ZONE2_MS;
                timer_active = add_repeating_timer_ms(period, timer_callback, NULL, &timer);
            }
        }
    }
}