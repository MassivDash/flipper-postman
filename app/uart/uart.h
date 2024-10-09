#ifndef UART_H
#define UART_H

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_gpio.h>
#include <furi_hal_serial.h>

// Macros and constants
#define HTTP_TAG "Postman"
#define http_tag "postman_app"
#define UART_CH (FuriHalSerialIdUsart)
#define TIMEOUT_DURATION_TICKS (2 * 1000)
#define BAUDRATE (115200)
#define RX_BUF_SIZE 1024

// Define GPIO pins for UART
extern GpioPin test_pins[2];

// Event Flags for UART Worker Thread
typedef enum {
  WorkerEvtStop = (1 << 0),
  WorkerEvtRxDone = (1 << 1),
} WorkerEvtFlags;

// Serial states
typedef enum {
  INACTIVE,
  IDLE,
  RECEIVING,
  SENDING,
  ISSUE,
} SerialState;

// Callback function type
typedef void (*Postman_Callback)(const char *line, void *context);

// Postman struct
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

// Function declarations
void get_timeout_timer_callback(void *context);
bool postman_send_data(Postman *postman, const char *data);

bool postman_init(Postman *postman, Postman_Callback callback, void *context);
void postman_free(Postman *postman);

#endif // UART_H