

#pragma once

#include <furi_hal.h>

typedef struct Uart Uart;

void uart_terminal_uart_set_handle_rx_data_cb(
    Uart *uart,
    void (*handle_rx_data_cb)(uint8_t *buf, size_t len, void *context));
void uart_terminal_uart_tx(Uart *uart, uint8_t *data, size_t len);
Uart *uart_terminal_uart_init(void *context);
void uart_terminal_uart_free(Uart *uart);
bool uart_terminal_uart_send_version_command(Uart *uart);
