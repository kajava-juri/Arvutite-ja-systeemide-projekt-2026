#include "common.h"
#include "main.h"
#include <pico/multicore.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/util/queue.h"
#include "pid_control.h"

// Pin definitions
#define ENA_PIN 2   // PWM speed control
#define IN1_PIN 3   // Direction A
#define IN2_PIN 4   // Direction B

#define MOTOR_MIN_PWM 60
#define MOTOR_MAX_PWM 100

// PWM frequency: 25 kHz @ 150 MHz sys clock (150,000,000 / 25,000 = 6000 cycles)
#define PWM_WRAP 5999

// Function prototypes
void motor_init(void);
void set_motor_speed(uint32_t percent);

static float min_max_scale(float value, float min, float max) {
    return (value - min) / (max - min);
}

queue_t sensor_data_q;

void core1_entry() {
    motor_init();

    SensorData last_sensor_data = {0};

    uint32_t target_speed = 50;
    float setpoint = 50.0f; // Pa
    float preverror = 0.0;
    
    // set_motor_speed(100);

    // sleep_ms(5000);
    char buffer[100];
    bool fresh = false;

    pid_ctrl_t pid = {0};
    pid_init(&pid, 1.0f, 0.1f, 0.05f, MOTOR_MIN_PWM, MOTOR_MAX_PWM); // Initialize PID controller with gains and output limits

    while (1) {
        fresh = queue_try_remove(&sensor_data_q, &last_sensor_data);
        if(fresh) {
            snprintf(buffer, sizeof(buffer), "Core 1: Received sensor data: %.2f at %u ms\n", last_sensor_data.data, last_sensor_data.updated_ms);
            uart_puts(UART_ID, buffer);
        }
        
        if (last_sensor_data.updated_ms == 0) {
            // No valid sensor data yet, skip PID control
            sleep_ms(100);
            continue;
        }

        float output = pid_controller(setpoint, last_sensor_data.data, &pid,
            to_ms_since_boot(get_absolute_time() / 1000) - last_sensor_data.updated_ms); // Call PID controller

        int percent = (int)(output);
        if (percent < MOTOR_MIN_PWM) percent = MOTOR_MIN_PWM;
        if (percent > MOTOR_MAX_PWM) percent = MOTOR_MAX_PWM;
        uint16_t level = (uint16_t)((min_max_scale(output, MOTOR_MIN_PWM, MOTOR_MAX_PWM) * PWM_WRAP) / 100);
        snprintf(buffer, sizeof(buffer), "Core 1: PID output: %.2f, Percent: %d, Level: %u\n", output, percent, level);
        uart_puts(UART_ID, buffer);

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
    if (percent < MOTOR_MIN_PWM) percent = MOTOR_MIN_PWM;
    if (percent > MOTOR_MAX_PWM) percent = MOTOR_MAX_PWM;
    
    uint16_t level = (uint16_t)((min_max_scale(percent, MOTOR_MIN_PWM, MOTOR_MAX_PWM) * PWM_WRAP) / 100);
    pwm_set_gpio_level(ENA_PIN, level);
}
