#include "common.h"
#include "main.h"
#include <pico/multicore.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/util/queue.h"

// Pin definitions
#define ENA_PIN 2   // PWM speed control
#define IN1_PIN 3   // Direction A
#define IN2_PIN 4   // Direction B

// PWM frequency: 25 kHz @ 150 MHz sys clock (150,000,000 / 25,000 = 6000 cycles)
#define PWM_WRAP 5999

// Function prototypes
void motor_init(void);
void set_motor_speed(uint32_t percent);

queue_t sensor_data_q;

void core1_entry() {
    motor_init();

    SensorData last_sensor_data = {0};

    uint32_t target_speed = 50;
    
    // set_motor_speed(100);

    // sleep_ms(5000);
    char buffer[100];
    bool fresh = false;
    while (1) {
        fresh = queue_try_remove(&sensor_data_q, &last_sensor_data);
        if(fresh) {
            snprintf(buffer, sizeof(buffer), "Core 1: Received sensor data: %.2f at %u ms\n", last_sensor_data.data, last_sensor_data.updated_ms);
            uart_puts(UART_ID, buffer);
        }

        // set_motor_speed(target_speed);

        sleep_ms(1000);
    }
}

// Initialize GPIOs and Hardware PWM
void motor_init(void) {
    // Set up direction pins
    gpio_init(IN1_PIN);
    gpio_set_dir(IN1_PIN, GPIO_OUT);
    gpio_init(IN2_PIN);
    gpio_set_dir(IN2_PIN, GPIO_OUT);

    // Hardcode to forward direction
    gpio_put(IN1_PIN, 0);
    gpio_put(IN2_PIN, 1);

    // Set up PWM
    gpio_set_function(ENA_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(ENA_PIN);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, PWM_WRAP);

    pwm_init(slice_num, &config, true);
    
    // Start with the motor off
    pwm_set_gpio_level(ENA_PIN, 0);
}

// Set speed (0-100) 
void set_motor_speed(uint32_t percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    uint16_t level = (uint16_t)((percent * PWM_WRAP) / 100);
    pwm_set_gpio_level(ENA_PIN, level);
}
