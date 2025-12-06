#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#define L_IN1 14
#define L_IN2 15
#define L_PWM 13
#define R_IN1 16
#define R_IN2 17
#define R_PWM 12
#define PWM_WRAP 12500 

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // Konfiguracja GPIO
    gpio_init(L_IN1); gpio_set_dir(L_IN1, GPIO_OUT);
    gpio_init(L_IN2); gpio_set_dir(L_IN2, GPIO_OUT);
    gpio_init(R_IN1); gpio_set_dir(R_IN1, GPIO_OUT);
    gpio_init(R_IN2); gpio_set_dir(R_IN2, GPIO_OUT);

    printf("Piny skonfigurowane.\n");
    while (true) { sleep_ms(1000); }
    return 0;
}