#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// SILNIK Lewy
#define L_IN1 10
#define L_IN2 11
#define L_PWM 14 

// SILNIK Prawy
#define R_IN1 12
#define R_IN2 13
#define R_PWM 15

// CZUJNIK ŚRODKOWY
#define TRIG_PIN 4
#define ECHO_PIN 5

#define LED_PIN 25
#define PWM_WRAP 12500 

void motor_init_pin(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    // Dzielnik zegara - zapobiega piszczeniu silników
    pwm_set_clkdiv(slice_num, 100.0f); 
    pwm_set_wrap(slice_num, PWM_WRAP);
    pwm_set_enabled(slice_num, true);
}

void set_motor(uint in1, uint in2, uint pwm_pin, int speed) {
    uint slice_num = pwm_gpio_to_slice_num(pwm_pin);
    uint channel = pwm_gpio_to_channel(pwm_pin);
    uint16_t duty = (uint16_t)(abs(speed) * PWM_WRAP / 100);
    if (speed > 0) { gpio_put(in1, 1); gpio_put(in2, 0); }
    else if (speed < 0) { gpio_put(in1, 0); gpio_put(in2, 1); }
    else { gpio_put(in1, 0); gpio_put(in2, 0); }
    pwm_set_chan_level(slice_num, channel, duty);
}

void drive(int left_speed, int right_speed) {
    set_motor(L_IN1, L_IN2, L_PWM, left_speed);
    set_motor(R_IN1, R_IN2, R_PWM, right_speed);
}

// Funkcja pomiaru odległości
float get_distance() {
    // 1. Wyslij impuls
    gpio_put(TRIG_PIN, 0); 
    sleep_us(2);
    gpio_put(TRIG_PIN, 1); 
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);

    // 2. Czekaj na start echa (z timeoutem 30ms)
    absolute_time_t timeout_time = make_timeout_time_ms(30);
    while (gpio_get(ECHO_PIN) == 0) {
        if (absolute_time_diff_us(get_absolute_time(), timeout_time) <= 0){
            return 0.0;
        }
    }

    // 3. Mierz czas trwania echa
    absolute_time_t start_time = get_absolute_time();
    while (gpio_get(ECHO_PIN) == 1) {
         if (absolute_time_diff_us(start_time, get_absolute_time()) > 30000) break; 
    }
    absolute_time_t end_time = get_absolute_time();
    
    return (float)absolute_time_diff_us(start_time, end_time) / 58.0f;
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Czas na start USB

    // Inicjalizacja GPIO
    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);
    
    // Silniki (Kierunek)
    gpio_init(L_IN1); gpio_set_dir(L_IN1, GPIO_OUT);
    gpio_init(L_IN2); gpio_set_dir(L_IN2, GPIO_OUT);
    gpio_init(R_IN1); gpio_set_dir(R_IN1, GPIO_OUT);
    gpio_init(R_IN2); gpio_set_dir(R_IN2, GPIO_OUT);
    
    // Czujnik
    gpio_init(TRIG_PIN); gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_init(ECHO_PIN); gpio_set_dir(ECHO_PIN, GPIO_IN);

    // Silniki (PWM)
    motor_init_pin(L_PWM);
    motor_init_pin(R_PWM);

    printf("Robot gotowy (Wersja 1 czujnik - Tabela Projektowa)\n");

    // Mrugnij na start
    for(int i=0; i<3; i++) {
        gpio_put(LED_PIN, 1); sleep_ms(200);
        gpio_put(LED_PIN, 0); sleep_ms(200);
    }

    while (true) {
        float dist = get_distance();
        printf("Dystans: %.1f cm\n", dist);

        if (dist <= 0 || dist > 400){
            continue; // BŁĄD POMIARU
        }
        

        if (dist > 0.1 && dist < 25.0) {
            // PRZESZKODA
            gpio_put(LED_PIN, 1);
            
            drive(0, 0);      
            sleep_ms(200); // Stop
            drive(-40, -40);  
            sleep_ms(400); // Tył
            drive(50, -50);
            sleep_ms(400); // Obrót (w prawo)
            drive(0, 0);      
            sleep_ms(200); // Stop
            
        } else {
            // DROGA WOLNA
            gpio_put(LED_PIN, 0);
            drive(35, 35); 
        }

        sleep_ms(60);
    }
}