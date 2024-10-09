#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_gpio.h>
#include <furi_hal_serial.h>

#define HTTP_TAG "Postman"
#define http_tag "postman_app"
#define UART_CH (FuriHalSerialIdUsart)
#define TIMEOUT_DURATION_TICKS (2 * 1000)
#define BAUDRATE (115200)
#define RX_BUF_SIZE 1024

// Define GPIO pins for UART
GpioPin test_pins[2] = {
    {.port = GPIOA, .pin = LL_GPIO_PIN_7}, // USART1_RX
    {.port = GPIOA, .pin = LL_GPIO_PIN_6}  // USART1_TX
};

// Event Flags for UART Worker Thread
typedef enum {
  WorkerEvtStop = (1 << 0),
  WorkerEvtRxDone = (1 << 1),
} WorkerEvtFlags;

typedef enum {
  INACTIVE,  // Inactive state
  IDLE,      // Default state
  RECEIVING, // Receiving data
  SENDING,   // Sending data
  ISSUE,     // Issue with connection
} SerialState;

typedef enum {
  CMD_SET_SSID,
  CMD_SET_PASSWORD,
  CMD_ACTIVATE_WIFI,
  CMD_DISCONNECT_WIFI,
  CMD_LIST_WIFI,
  CMD_GET,
  CMD_GET_STREAM,
  CMD_POST,
  CMD_BUILD_HTTP_METHOD,
  CMD_BUILD_HTTP_URL,
  CMD_BUILD_HTTP_HEADER,
  CMD_BUILD_HTTP_PAYLOAD,
  CMD_REMOVE_HTTP_HEADER,
  CMD_RESET_HTTP_CONFIG,
  CMD_BUILD_HTTP_SHOW_RESPONSE_HEADERS,
  CMD_BUILD_HTTP_IMPLEMENTATION,
  CMD_EXECUTE_HTTP_CALL,
  CMD_CONNECT,
  CMD_HELP,
  CMD_UNKNOWN
} CommandType;

typedef enum {
  RESP_WIFI_CONNECTED,
  RESP_WIFI_DISCONNECTED,
  RESP_WIFI_SSID,
  RESP_WIFI_PASSWORD,
  RESP_WIFI_LIST,
  RESP_HTTP_GET,
  RESP_HTTP_GET_STREAM,
  RESP_HTTP_POST,
  RESP_HTTP_METHOD,
  RESP_HTTP_URL,
  RESP_HTTP_HEADER,
  RESP_HTTP_PAYLOAD,
  RESP_HTTP_REMOVE_HEADER,
  RESP_HTTP_RESET_CONFIG,
  RESP_HTTP_SHOW_RESPONSE_HEADERS,
  RESP_HTTP_IMPLEMENTATION,
  RESP_HTTP_EXECUTE_CALL,
  RESP_HELP,
  RESP_UNKNOWN
} ResponseType;

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

Postman postman;

void get_timeout_timer_callback(void *context) {
  Postman *postman = (Postman *)context;
  furi_assert(postman);
  FURI_LOG_E(HTTP_TAG,
             "Timeout reached: 2 seconds without receiving [GET/END]...");

  // Reset the state
  postman->started_receiving = false;

  // Free received data if any
  if (postman->received_data) {
    free(postman->received_data);
    postman->received_data = NULL;
  }

  // Update UART state
  postman->state = ISSUE;
}

// UART RX Handler Callback (Interrupt Context)
static void _postman_rx_callback(FuriHalSerialHandle *handle,
                                 FuriHalSerialRxEvent event, void *context) {
  Postman *postman = (Postman *)context;
  furi_assert(postman);
  if (event == FuriHalSerialRxEventData) {
    uint8_t data = furi_hal_serial_async_rx(handle);
    furi_stream_buffer_send(postman->postman_stream, &data, 1, 0);
    furi_thread_flags_set(postman->rx_thread_id, WorkerEvtRxDone);
  }
}

// UART worker thread
static int32_t postman_worker(void *context) {
  Postman *postman = (Postman *)context;
  furi_assert(postman);
  size_t rx_line_pos = 0;
  char rx_line_buffer[256]; // Buffer to collect a line

  while (1) {
    uint32_t events = furi_thread_flags_wait(WorkerEvtStop | WorkerEvtRxDone,
                                             FuriFlagWaitAny, FuriWaitForever);
    if (events & WorkerEvtStop)
      break;
    if (events & WorkerEvtRxDone) {
      size_t len = furi_stream_buffer_receive(
          postman->postman_stream, postman->rx_buf, RX_BUF_SIZE * 10, 0);
      for (size_t i = 0; i < len; i++) {
        char c = postman->rx_buf[i];
        if (c == '\n' || rx_line_pos >= sizeof(rx_line_buffer) - 1) {
          rx_line_buffer[rx_line_pos] = '\0';
          // Invoke the callback with the complete line
          if (postman->handle_rx_line_cb) {
            postman->handle_rx_line_cb(rx_line_buffer,
                                       postman->callback_context);
          }
          // Reset the line buffer
          rx_line_pos = 0;
        } else {
          rx_line_buffer[rx_line_pos++] = c;
        }
      }
    }
  }

  return 0;
}

bool postman_send_data(Postman *postman, const char *data) {
  size_t data_length = strlen(data);
  if (data_length == 0) {
    FURI_LOG_E("Postman", "Attempted to send empty data.");
    return false;
  }

  size_t send_length = data_length + 1;
  if (send_length > 256) {
    FURI_LOG_E("Postman", "Data too long to send over Postman.");
    return false;
  }

  char send_buffer[257];
  strncpy(send_buffer, data, 256);
  send_buffer[data_length] = '\n';
  send_buffer[data_length + 1] = '\0';

  if (postman->state == INACTIVE &&
      ((strstr(send_buffer, "[PING]") == NULL) &&
       (strstr(send_buffer, "[WIFI/CONNECT]") == NULL))) {
    FURI_LOG_E("Postman", "Cannot send data while INACTIVE.");
    postman->last_response = "Cannot send data while INACTIVE.";
    return false;
  }

  postman->state = SENDING;
  furi_hal_serial_tx(postman->serial_handle, (const uint8_t *)send_buffer,
                     send_length);
  postman->state = IDLE;
  return true;
}

bool postman_init(Postman *postman, Postman_Callback callback, void *context) {
  postman->postman_stream = furi_stream_buffer_alloc(RX_BUF_SIZE, 1);
  if (!postman->postman_stream) {
    FURI_LOG_E(HTTP_TAG, "Failed to allocate UART stream buffer.");
    return false;
  }

  postman->rx_thread = furi_thread_alloc();
  if (!postman->rx_thread) {
    FURI_LOG_E(HTTP_TAG, "Failed to allocate UART thread.");
    furi_stream_buffer_free(postman->postman_stream);
    return false;
  }

  furi_thread_set_name(postman->rx_thread, "Postman_RxThread");
  furi_thread_set_stack_size(postman->rx_thread, 1024);
  furi_thread_set_context(postman->rx_thread, postman);
  furi_thread_set_callback(postman->rx_thread, postman_worker);

  postman->handle_rx_line_cb = callback;
  postman->callback_context = context;

  furi_thread_start(postman->rx_thread);
  postman->rx_thread_id = furi_thread_get_id(postman->rx_thread);

  furi_hal_gpio_init_simple(&test_pins[0], GpioModeInput);
  furi_hal_gpio_init_simple(&test_pins[1], GpioModeOutputPushPull);

  postman->serial_handle = furi_hal_serial_control_acquire(UART_CH);
  if (postman->serial_handle == NULL) {
    FURI_LOG_E(HTTP_TAG, "Failed to acquire UART control - handle is NULL");
    furi_thread_free(postman->rx_thread);
    furi_stream_buffer_free(postman->postman_stream);
    return false;
  }

  furi_hal_serial_init(postman->serial_handle, BAUDRATE);
  furi_hal_serial_enable_direction(postman->serial_handle,
                                   FuriHalSerialDirectionRx);
  furi_hal_serial_async_rx_start(postman->serial_handle, _postman_rx_callback,
                                 postman, false);
  furi_hal_serial_tx_wait_complete(postman->serial_handle);

  postman->get_timeout_timer =
      furi_timer_alloc(get_timeout_timer_callback, FuriTimerTypeOnce, postman);
  if (!postman->get_timeout_timer) {
    FURI_LOG_E(HTTP_TAG, "Failed to allocate GET timeout timer.");
    furi_hal_serial_async_rx_stop(postman->serial_handle);
    furi_hal_serial_disable_direction(postman->serial_handle,
                                      FuriHalSerialDirectionRx);
    furi_hal_serial_control_release(postman->serial_handle);
    furi_hal_serial_deinit(postman->serial_handle);
    furi_thread_flags_set(postman->rx_thread_id, WorkerEvtStop);
    furi_thread_join(postman->rx_thread);
    furi_thread_free(postman->rx_thread);
    furi_stream_buffer_free(postman->postman_stream);
    return false;
  }

  furi_timer_set_thread_priority(FuriTimerThreadPriorityElevated);

  FURI_LOG_I(HTTP_TAG, "UART initialized successfully.");
  return true;
}

void postman_free(Postman *postman) {
  if (postman->serial_handle == NULL) {
    FURI_LOG_E(HTTP_TAG, "UART handle is NULL. Already deinitialized?");
    return;
  }
  furi_hal_serial_async_rx_stop(postman->serial_handle);
  furi_hal_serial_disable_direction(postman->serial_handle,
                                    FuriHalSerialDirectionRx);
  furi_hal_serial_control_release(postman->serial_handle);
  furi_hal_serial_deinit(postman->serial_handle);
  furi_thread_flags_set(postman->rx_thread_id, WorkerEvtStop);
  furi_thread_join(postman->rx_thread);
  furi_thread_free(postman->rx_thread);
  furi_stream_buffer_free(postman->postman_stream);
  if (postman->get_timeout_timer) {
    furi_timer_free(postman->get_timeout_timer);
    postman->get_timeout_timer = NULL;
  }
  if (postman->received_data) {
    free(postman->received_data);
    postman->received_data = NULL;
  }
  FURI_LOG_I("Postman", "UART deinitialized successfully.");
}