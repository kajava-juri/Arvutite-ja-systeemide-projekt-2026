#include "common.h"
#include <string.h>
#include <stdio.h>
#include "main.h"
#include <pico/multicore.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/util/queue.h"
#include "pid_control.h"
#include "devices/modbus_interface.h"
#include "pico/flash.h"
#include "hardware/flash.h"
#include <inttypes.h>
#include "hardware/clocks.h"

// Pin definitions
#define ENA_PIN 6   // PWM speed control
#define IN1_PIN 7   // Direction A
#define IN2_PIN 8   // Direction B

#define PWM_FREQUENCY_HZ 25000

// Last 4 KB sector of flash for data storage
#define DATA_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define ENCODER_SAVE_DELAY_MS 10000
#define STORAGE_MAGIC 0xDEADBEEF // random bits to check on read that data was written by us and not random flash data

typedef struct {
    uint32_t magic_header; 
    uint32_t encoder_value; 
} encoder_storage_t;

static int previous_value = 1;
static uint32_t rotary_value = 0; // Start at max speed
static bool save_pending = false;
static absolute_time_t save_deadline;
static uint32_t pwm_wrap;

static const volatile encoder_storage_t *saved_data =
    (const volatile encoder_storage_t *)(XIP_BASE + DATA_FLASH_OFFSET);

// Function prototypes
void motor_init(void);
void set_motor_speed(uint32_t percent);

static float min_max_scale(float value, float min, float max) {
    return (value - min) / (max - min);
}

// Worker function for flash_safe_execute
static void flash_program_worker(void *param) {
    // Erase 4KB sector and program 256-byte page
    // byte count must be a multiple of 256 as per pico sdk documentation
    uint8_t page_buffer[FLASH_PAGE_SIZE];
    // programming flash can only change bits from 1 to 0
    // to not waste programming unused space, set all bits in buffer to 1 before copyng the data
    memset(page_buffer, 0xFF, sizeof(page_buffer));
    memcpy(page_buffer, param, sizeof(encoder_storage_t));

    flash_range_erase(DATA_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(DATA_FLASH_OFFSET, page_buffer, FLASH_PAGE_SIZE);
}

// Write to flash via SDK safe wrapper
bool save_encoder_value(uint32_t value) {
    if (saved_data->magic_header == STORAGE_MAGIC &&
        saved_data->encoder_value == value) {
        save_pending = false;
        return true;
    }
    encoder_storage_t data_to_save;
    data_to_save.magic_header = STORAGE_MAGIC;
    data_to_save.encoder_value = value;

    int status = flash_safe_execute(flash_program_worker, &data_to_save, 100);
    return (status == PICO_OK);
}

uint32_t load_encoder_value(uint32_t default_value) {
    if (saved_data->magic_header == STORAGE_MAGIC &&
        saved_data->encoder_value <= 40) {
        printf("Restored encoder value: %" PRIu32 "\n", saved_data->encoder_value);
        return (uint32_t)saved_data->encoder_value;
    } else {
        printf("No valid flash data (magic 0x%08" PRIX32"). Using default: %" PRIu32 "\n", 
        saved_data->magic_header,default_value);
        return default_value;
    }
}

static void mark_rotary_changed(void) {
    save_pending = true;
    save_deadline = make_timeout_time_ms(ENCODER_SAVE_DELAY_MS);
}

static void process_flash_save(void) {
    if (save_pending && time_reached(save_deadline)) {
        save_pending = false;
        save_encoder_value(rotary_value);
    }
}

static void rotary_changed(void) {
    int clk = gpio_get(CLK_PIN);   // read once so all checks see the same state

    if (previous_value != clk) {
        if (clk == 0) {
            if (gpio_get(DT_PIN) == 0) {
                if (rotary_value > 0) rotary_value--;
                mark_rotary_changed();
            } else {
                if (rotary_value < 40) rotary_value++;
                mark_rotary_changed();
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

static void publish_setpoint_to_core0(uint32_t setpoint) {
    queue_try_add(&core1_to_core0_q, &setpoint);
}

void core1_entry() {
    motor_init();

    rotary_value = load_encoder_value(0);

    SensorData last_sensor_data = {0};

    // sleep_ms(5000);
    char buffer[100];
    bool fresh = false;

    pid_ctrl_t pid = {0};
    pid_init(&pid, 0.3f, 0.1f, 0.05f, 0, 100); // Initialize PID controller with gains and output limits

    absolute_time_t next_print = make_timeout_time_ms(100);
    absolute_time_t next_motor_update = make_timeout_time_ms(1000);
    absolute_time_t queue_poll_time = make_timeout_time_ms(100);

    while (1) {
        
        rotary_changed();
        
        // Check if flash write delay timer expired
        process_flash_save();
        
        if (time_reached(queue_poll_time)) {
            queue_poll_time = make_timeout_time_ms(100);
            fresh = queue_try_remove(&sensor_data_q, &last_sensor_data);
            if(fresh) {
                snprintf(buffer, sizeof(buffer), "Core 1: Received sensor data: %.2f at %u ms\n", last_sensor_data.data, last_sensor_data.updated_ms);
                uart_puts(UART_ID, buffer);
            }

            publish_setpoint_to_core0(rotary_value);
            
            if (last_sensor_data.updated_ms == 0) {
                // No valid sensor data yet, skip PID control
                sleep_ms(100);
                continue;
            }

            float output = pid_controller((float)rotary_value, last_sensor_data.data, &pid, (float)queue_poll_time / 1000.0f);
        }
        
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

        set_motor_speed((uint32_t)GET_PWM_NORMALIZED(min_max_scale(pid.output, pid.output_min, pid.output_max)));

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
    pwm_wrap = (clock_get_hz(clk_sys) / PWM_FREQUENCY_HZ) - 1;

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, pwm_wrap);

    pwm_init(slice_num, &config, true);
    
    // Start with the motor off
    pwm_set_gpio_level(ENA_PIN, 0);
}

// Set speed (0-100) 
void set_motor_speed(uint32_t percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    uint16_t level = (uint16_t)((percent * pwm_wrap) / 100);
    pwm_set_gpio_level(ENA_PIN, level);
}
