#include "uart.h"
#include "../app.h"
#include "../structs.h"
#include "../version.h"
#include <furi.h>
#include <furi_hal.h>

#define UART_CH (FuriHalSerialIdUsart) // UART channel

struct Uart {
  App *app;
  FuriThread *rx_thread;
  FuriStreamBuffer *rx_stream;
  uint8_t rx_buf[RX_BUF_SIZE + 1];
  void (*handle_rx_data_cb)(uint8_t *buf, size_t len, void *context);
  FuriHalSerialHandle *serial_handle;
  char last_response[1024]; // Buffer to store the last response
};

typedef enum {
  WorkerEvtStop = (1 << 0),
  WorkerEvtRxDone = (1 << 1),
} WorkerEvtFlags;

void uart_terminal_uart_set_handle_rx_data_cb(
    Uart *uart,
    void (*handle_rx_data_cb)(uint8_t *buf, size_t len, void *context)) {
  furi_assert(uart);
  uart->handle_rx_data_cb = handle_rx_data_cb;
}

#define WORKER_ALL_RX_EVENTS (WorkerEvtStop | WorkerEvtRxDone)

void uart_terminal_uart_on_irq_cb(FuriHalSerialHandle *handle,
                                  FuriHalSerialRxEvent event, void *context) {
  Uart *uart = (Uart *)context;

  if (event == FuriHalSerialRxEventData) {
    uint8_t data = furi_hal_serial_async_rx(handle);
    furi_stream_buffer_send(uart->rx_stream, &data, 1, 0);
    furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtRxDone);
  }
}

static int32_t uart_worker(void *context) {
  Uart *uart = (Uart *)context;
  size_t response_len = 0;

  while (1) {
    uint32_t events = furi_thread_flags_wait(WORKER_ALL_RX_EVENTS,
                                             FuriFlagWaitAny, FuriWaitForever);
    furi_check((events & FuriFlagError) == 0);
    if (events & WorkerEvtStop)
      break;
    if (events & WorkerEvtRxDone) {
      size_t len = furi_stream_buffer_receive(uart->rx_stream, uart->rx_buf,
                                              RX_BUF_SIZE, 0);
      if (len > 0) {
        uart->rx_buf[len] = '\0'; // Null-terminate the received data

        // Accumulate the response
        if (response_len + len < sizeof(uart->last_response)) {
          strncpy(uart->last_response + response_len, (char *)uart->rx_buf,
                  len);
          response_len += len;
          uart->last_response[response_len] = '\0'; // Ensure null-termination

          // Check if the response contains a newline character
          if (strchr(uart->last_response, '\n')) {
            if (uart->handle_rx_data_cb) {
              uart->handle_rx_data_cb((uint8_t *)uart->last_response,
                                      response_len, uart->app);
            }
            response_len = 0; // Reset for the next response
          }
        } else {
          FURI_LOG_E("UART", "Response buffer overflow.");
          response_len = 0; // Reset to avoid overflow
        }
      }
    }
  }

  furi_stream_buffer_free(uart->rx_stream);

  return 0;
}

void uart_terminal_uart_tx(Uart *uart, uint8_t *data, size_t len) {
  if (!uart || !uart->serial_handle) {
    FURI_LOG_E("UART", "Invalid UART or serial handle.");
    return;
  }
  furi_hal_serial_tx(uart->serial_handle, data, len);
}

Uart *uart_terminal_uart_init(void *context) {
  App *app = context;
  if (!app) {
    FURI_LOG_E("UART", "Invalid app context.");
    return NULL;
  }

  Uart *uart = malloc(sizeof(Uart));
  if (!uart) {
    FURI_LOG_E("UART", "Failed to allocate memory for UART.");
    return NULL;
  }

  uart->app = app;
  uart->rx_stream = furi_stream_buffer_alloc(RX_BUF_SIZE, 1);
  if (!uart->rx_stream) {
    FURI_LOG_E("UART", "Failed to allocate RX stream buffer.");
    free(uart);
    return NULL;
  }

  uart->rx_thread = furi_thread_alloc();
  if (!uart->rx_thread) {
    FURI_LOG_E("UART", "Failed to allocate RX thread.");
    furi_stream_buffer_free(uart->rx_stream);
    free(uart);
    return NULL;
  }

  furi_thread_set_name(uart->rx_thread, "UART_TerminalUartRxThread");
  furi_thread_set_stack_size(uart->rx_thread, 512);
  furi_thread_set_context(uart->rx_thread, uart);
  furi_thread_set_callback(uart->rx_thread, uart_worker);

  furi_thread_start(uart->rx_thread);

  uart->serial_handle = furi_hal_serial_control_acquire(app->uart_ch);
  if (!uart->serial_handle) {
    FURI_LOG_E("UART", "Failed to acquire serial handle.");
    furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtStop);
    furi_thread_join(uart->rx_thread);
    furi_thread_free(uart->rx_thread);
    furi_stream_buffer_free(uart->rx_stream);
    free(uart);
    return NULL;
  }

  furi_hal_serial_init(uart->serial_handle, 115200);
  furi_hal_serial_async_rx_start(uart->serial_handle,
                                 uart_terminal_uart_on_irq_cb, uart, false);

  return uart;
}

bool uart_terminal_uart_send_version_command(Uart *uart) {
  if (!uart) {
    FURI_LOG_E("UART", "Invalid UART context.");
    return false;
  }

  const char *version_command = "VERSION\n";
  uart_terminal_uart_tx(uart, (uint8_t *)version_command,
                        strlen(version_command));

  // Wait for the response
  uint32_t events =
      furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, 2000);
  if (events & WorkerEvtRxDone) {
    // Split the response at the newline character
    char *newline_pos = strchr(uart->last_response, '\n');
    if (newline_pos) {
      *newline_pos = '\0'; // Terminate the string at the newline character
    }

    char version_check[50];
    snprintf(version_check, sizeof(version_check), "VERSION: %s",
             MIN_BOARD_VERSION);
    if (strstr(uart->last_response, version_check)) {
      return true;
    }
  }
  FURI_LOG_E("UART", "No response received from the board.");
  return false;
}

void uart_terminal_uart_free(Uart *uart) {
  if (!uart) {
    FURI_LOG_E("UART", "Invalid UART context.");
    return;
  }

  if (uart->serial_handle) {
    furi_hal_serial_async_rx_stop(uart->serial_handle);
    furi_hal_serial_deinit(uart->serial_handle);
    furi_hal_serial_control_release(uart->serial_handle);
  }

  if (uart->rx_thread) {
    furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtStop);
    furi_thread_join(uart->rx_thread);
    furi_thread_free(uart->rx_thread);
  }

  if (uart->rx_stream) {
    furi_stream_buffer_free(uart->rx_stream);
  }

  free(uart);
}