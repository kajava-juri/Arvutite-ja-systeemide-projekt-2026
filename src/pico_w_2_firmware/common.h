#ifndef COMMON_H
#define COMMON_H

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart0
#define UART_ID_MAX485 uart1
#define BAUD_RATE 9600 // 115200
#define BAUD_RATE_MAX485 9600

// Use pins 0 and 1 for UART0
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 0
#define UART_RX_PIN 1

#define UART_TX_PIN_MAX485 8
#define UART_RX_PIN_MAX485 9

#define RE_DE_PIN 15

#endif // COMMON_H