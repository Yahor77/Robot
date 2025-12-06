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

#define TRIG_F 18
#define ECHO_F 19
#define TRIG_L 20
#define ECHO_L 21
#define TRIG_R 22
#define ECHO_R 26

#define LED_PIN 25

#include <stdlib.h> // Dodane dla abs()

void motor_init_pin(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
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

float get_distance(uint trig, uint echo) {

    gpio_put(trig, 0);
    sleep_us(2);
    gpio_put(trig, 1);
    sleep_us(10);
    gpio_put(trig, 0);

    absolute_time_t timeout_time = make_timeout_time_ms(30);

    while (gpio_get(echo) == 0) {
        if (absolute_time_diff_us(get_absolute_time(), timeout_time) < 0) {
            return 0.0;
        }
    }

    absolute_time_t start_time = get_absolute_time();

    while (gpio_get(echo) == 1) {
         if (absolute_time_diff_us(start_time, get_absolute_time()) > 30000){
            break; 
         }
    }

    absolute_time_t end_time = get_absolute_time();
    
    
    int64_t pulse_duration = absolute_time_diff_us(start_time, end_time);
    float distance = (float)pulse_duration / 58.0f;
    
    if (distance > 400.0){
        return 0.0;
    }

    return distance;
}


int main() {
    stdio_init_all();
    sleep_ms(2000);
    gpio_init(L_IN1); gpio_set_dir(L_IN1, GPIO_OUT);
    gpio_init(L_IN2); gpio_set_dir(L_IN2, GPIO_OUT);
    gpio_init(R_IN1); gpio_set_dir(R_IN1, GPIO_OUT);
    gpio_init(R_IN2); gpio_set_dir(R_IN2, GPIO_OUT);

    motor_init_pin(L_PWM);
    motor_init_pin(R_PWM);
    
    while (true) { sleep_ms(1000); }
    return 0;
    return 0;
}