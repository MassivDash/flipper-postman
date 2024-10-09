#ifndef UART_UTILS_H
#define UART_UTILS_H

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_gpio.h>
#include <furi_hal_serial.h>
#include <storage/storage.h>

#define HTTP_TAG "Postman"
#define http_tag "postman_app"
#define UART_CH (FuriHalSerialIdUsart)
#define TIMEOUT_DURATION_TICKS (2 * 1000)
#define BAUDRATE (115200)
#define RX_BUF_SIZE 1024

typedef enum {
  INACTIVE,  // Inactive state
  IDLE,      // Default state
  RECEIVING, // Receiving data
  SENDING,   // Sending data
  ISSUE,     // Issue with connection
} SerialState;

typedef void (*Postman_Callback)(const char *line, void *context);

typedef struct {
  FuriStreamBuffer *postman_stream;
  FuriHalSerialHandle *serial_handle;
  FuriThread *rx_thread;
  uint8_t rx_buf[RX_BUF_SIZE];
  FuriThreadId rx_thread_id;
  Postman_Callback handle_rx_line_cb;
  void *callback_context;
  SerialState state;
  char *last_response;
  FuriTimer *get_timeout_timer;
  bool started_receiving;
  bool just_started;
  char *received_data;
} Postman;

bool postman_init(Postman *postman, Postman_Callback callback, void *context);
void postman_deinit(Postman *postman);
bool postman_send_command_and_check_response(Postman *postman);

#endif // UART_UTILS_H