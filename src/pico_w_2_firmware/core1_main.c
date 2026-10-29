#include "common.h"
#include "main.h"
#include <pico/multicore.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/util/queue.h"
#include "pid_control.h"
#include "devices/modbus_interface.h"

// Pin definitions
#define ENA_PIN 6   // PWM speed control
#define IN1_PIN 7   // Direction A
#define IN2_PIN 8   // Direction B

// PWM frequency: 25 kHz @ 150 MHz sys clock (150,000,000 / 25,000 = 6000 cycles)
#define PWM_WRAP 5999

static int previous_value = 1;
static uint16_t rotary_value = MOTOR_MAX_PWM; // Start at max speed

// Function prototypes
void motor_init(void);
void set_motor_speed(uint32_t percent);

static float min_max_scale(float value, float min, float max) {
    return (value - min) / (max - min);
}

static void rotary_changed(void) {
    int clk = gpio_get(CLK_PIN);   // read once so all checks see the same state

    if (previous_value != clk) {
        if (clk == 0) {
            if (gpio_get(DT_PIN) == 0) {
                if (rotary_value > MOTOR_MIN_PWM) rotary_value--;
            } else {
                if (rotary_value < MOTOR_MAX_PWM) rotary_value++;
            }
        }
        previous_value = clk;
    }

    if (gpio_get(SW_PIN) == 0) {
        // printf("Button pressed\n");
        sleep_ms(1000);
    }
}

queue_t sensor_data_q;
queue_t core1_to_core0_q;

static void publish_setpoint_to_core0(uint16_t setpoint) {
    queue_try_add(&core1_to_core0_q, &setpoint);
}

void core1_entry() {
    motor_init();

    SensorData last_sensor_data = {0};

    uint32_t target_speed = 50;
    float preverror = 0.0;
    
    // set_motor_speed(100);

    // sleep_ms(5000);
    char buffer[100];
    bool fresh = false;

    // pid_ctrl_t pid = {0};
    // pid_init(&pid, 1.0f, 0.1f, 0.05f, MOTOR_MIN_PWM, MOTOR_MAX_PWM); // Initialize PID controller with gains and output limits

    absolute_time_t next_print = make_timeout_time_ms(100);
    absolute_time_t next_motor_update = make_timeout_time_ms(1000);
    absolute_time_t queue_poll_time = make_timeout_time_ms(100);

    while (1) {
        if (time_reached(queue_poll_time)) {
            queue_poll_time = make_timeout_time_ms(100);
            fresh = queue_try_remove(&sensor_data_q, &last_sensor_data);
            if(fresh) {
                snprintf(buffer, sizeof(buffer), "Core 1: Received sensor data: %.2f at %u ms\n", last_sensor_data.data, last_sensor_data.updated_ms);
                uart_puts(UART_ID, buffer);
            }

            publish_setpoint_to_core0((uint16_t)rotary_value);
            
            if (last_sensor_data.updated_ms == 0) {
                // No valid sensor data yet, skip PID control
                sleep_ms(100);
                continue;
            }

            // float output = pid_controller(setpoint, last_sensor_data.data, &pid,
            //     to_ms_since_boot(get_absolute_time() / 1000) - last_sensor_data.updated_ms); // Call PID controller

            // int percent = (int)(output);
            // if (percent < MOTOR_MIN_PWM) percent = MOTOR_MIN_PWM;
            // if (percent > MOTOR_MAX_PWM) percent = MOTOR_MAX_PWM;
            // uint16_t level = (uint16_t)((min_max_scale(output, MOTOR_MIN_PWM, MOTOR_MAX_PWM) * PWM_WRAP) / 100);
            // snprintf(buffer, sizeof(buffer), "Core 1: PID output: %.2f, Percent: %d, Level: %u\n", output, percent, level);
            // uart_puts(UART_ID, buffer);
        }

        rotary_changed();

        if (time_reached(next_print)) {
            char buffer[100];
            snprintf(buffer, sizeof(buffer), "Rotary encoder value: %d\n", rotary_value);
            uart_puts(UART_ID, buffer);
            next_print = make_timeout_time_ms(100);
        }

        if (time_reached(next_motor_update)) {
            set_motor_speed(rotary_value);
            next_motor_update = make_timeout_time_ms(1000);
        }

        // set_motor_speed(rotary_value);

        sleep_us(500);
    }
}

static void init_input(uint pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
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

    init_input(CLK_PIN);
    init_input(DT_PIN);
    init_input(SW_PIN);

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
