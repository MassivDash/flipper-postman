#include "uart.h"
#include "../app.h"
#include "../structs.h"
#include "../version.h"
#include <furi.h>
#include <furi_hal.h>
#include "../scene/download/download.h"

#define UART_CH (FuriHalSerialIdUsart) // UART channel

struct Uart {
    App* app;
    FuriThread* rx_thread;
    FuriStreamBuffer* rx_stream;
    uint8_t rx_buf[RX_BUF_SIZE + 1];
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context);
    FuriHalSerialHandle* serial_handle;
    char last_response[LAST_RESPONSE_SIZE]; // Buffer to store the last response
    bool streaming;
    FuriTimer* timer;
    size_t bytes_written;
};

void uart_terminal_uart_set_handle_rx_data_cb(
    Uart* uart,
    void (*handle_rx_data_cb)(uint8_t* buf, size_t len, void* context)) {
    furi_assert(uart);
    uart->handle_rx_data_cb = handle_rx_data_cb;
}

#define WORKER_ALL_RX_EVENTS (WorkerEvtStop | WorkerEvtRxDone | WorkerEvtRxIdle)

void uart_terminal_uart_on_irq_cb(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent event,
    void* context) {
    Uart* uart = (Uart*)context;

    if(event == FuriHalSerialRxEventData) {
        uint8_t data = furi_hal_serial_async_rx(handle);
        furi_stream_buffer_send(uart->rx_stream, &data, sizeof(data), 0);
        furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtRxDone);
    }

    if(event == FuriHalSerialRxEventIdle) {
        furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtRxIdle);
    }
}

static void handle_save_to_file(Uart* uart, App* app, size_t* len) {
    if(*len > 0) {
        FuriString* full_path = furi_string_alloc();
        furi_string_printf(full_path, "%s/%s", APP_DATA_PATH(""), app->filename);

        File* file = storage_file_alloc(app->storage);
        if(storage_file_open(file, furi_string_get_cstr(full_path), FSAM_WRITE, FSOM_OPEN_APPEND)) {
            if(!storage_file_write(file, uart->rx_buf, *len)) {
                FURI_LOG_E(TAG, "DOWNLOAD_ERROR: Failed to write to file");
                furi_string_cat_str(
                    app->text_box_store, "DOWNLOAD_ERROR: Failed to write to file\n");
            } else {
                static size_t total_written = 0;
                total_written += *len;
                uart->bytes_written = total_written;
                update_download_progress(app, total_written);
            }
            storage_file_close(file);
        } else {
            FURI_LOG_E(TAG, "DOWNLOAD_ERROR: Failed to open file for writing");
            furi_string_cat_str(
                app->text_box_store, "DOWNLOAD_ERROR: Failed to open file for writing\n");
            uart->bytes_written = 0;
            update_download_progress(app, 0);
        }
        storage_file_free(file);
        furi_string_free(full_path);
    }
}

static void handle_full_response(Uart* uart, App* app, size_t* len) {
    uart->rx_buf[*len] = '\0'; // Null-terminate the received data
    furi_string_cat_str(app->text_box_store, (char*)uart->rx_buf);
}

static void handle_last_response(Uart* uart, App* app, size_t* response_len, size_t* len) {
    UNUSED(app);
    uart->rx_buf[*len] = '\0'; // Null-terminate the received data
    if(*response_len + *len < sizeof(uart->last_response)) {
        strncpy(uart->last_response + *response_len, (char*)uart->rx_buf, *len);
        *response_len += *len;
        uart->last_response[*response_len] = '\0'; // Ensure null-termination

        // Check if the response contains a newline character
        if(strchr(uart->last_response, '\n')) {
            if(uart->handle_rx_data_cb) {
                uart->handle_rx_data_cb((uint8_t*)uart->last_response, *response_len, uart->app);
            }
            *response_len = 0; // Reset for the next response
        }
    } else {
        FURI_LOG_E(TAG, "HTTP_RESPONSE_ERROR: Response buffer overflow.");
        *response_len = 0; // Reset to avoid overflow
        uart->last_response[0] = '\0'; // Clear the buffer
    }
}

static int32_t uart_worker(void* context) {
    Uart* uart = (Uart*)context;
    App* app = uart->app;
    size_t response_len = 0;

    while(1) {
        bool save_to_file = app->save_to_file && app->filename[0] != '\0';
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_RX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        furi_check((events & FuriFlagError) == 0);
        if(events & WorkerEvtStop) break;
        if(events & WorkerEvtRxDone) {
            size_t len = furi_stream_buffer_receive(uart->rx_stream, uart->rx_buf, RX_BUF_SIZE, 0);
            if(len > 0) {
                if(save_to_file) {
                    handle_save_to_file(uart, app, &len);
                } else if(app->full_response) {
                    handle_full_response(uart, app, &len);
                } else {
                    handle_last_response(uart, app, &response_len, &len);
                }
            }
        }
    }

    furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtStop);
    furi_stream_buffer_reset(uart->rx_stream);
    furi_stream_buffer_free(uart->rx_stream);
    return 0;
}

bool uart_terminal_uart_tx(Uart* uart, uint8_t* data, size_t len) {
    FURI_LOG_D("UART", "Command: %s", data);
    if(!uart || !uart->serial_handle) {
        FURI_LOG_E(TAG, "Invalid UART or serial handle.");
        return false;
    }
    furi_hal_serial_tx(uart->serial_handle, data, len);
    return true;
}

Uart* uart_terminal_uart_init(void* context) {
    App* app = context;

    Uart* uart = malloc(sizeof(Uart));

    uart->app = app;
    uart->rx_stream = furi_stream_buffer_alloc(RX_BUF_SIZE + 1, 1);
    uart->rx_thread = furi_thread_alloc();
    uart->streaming = false;
    uart->bytes_written = 0;

    furi_thread_set_name(uart->rx_thread, "UART_TerminalUartRxThread");
    furi_thread_set_stack_size(uart->rx_thread, 2096);
    furi_thread_set_context(uart->rx_thread, uart);
    furi_thread_set_callback(uart->rx_thread, uart_worker);

    furi_thread_start(uart->rx_thread);

    uart->serial_handle = furi_hal_serial_control_acquire(app->uart_ch);
    if(!uart->serial_handle) {
        FURI_LOG_E(TAG, "UART_ERROR: Failed to acquire serial handle.");
        furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtStop);
        furi_thread_join(uart->rx_thread);
        furi_thread_free(uart->rx_thread);
        furi_stream_buffer_free(uart->rx_stream);
        free(uart);
        return NULL;
    }

    furi_hal_serial_init(uart->serial_handle, 115200);
    furi_hal_serial_async_rx_start(uart->serial_handle, uart_terminal_uart_on_irq_cb, uart, false);

    return uart;
}

void uart_terminal_uart_free(Uart* uart) {
    if(!uart) {
        FURI_LOG_E(TAG, "UART: Invalid UART context.");
        return;
    }

    if(uart->serial_handle) {
        furi_hal_serial_async_rx_stop(uart->serial_handle);
        furi_hal_serial_deinit(uart->serial_handle);
        furi_hal_serial_control_release(uart->serial_handle);
    }

    if(uart->rx_thread) {
        furi_thread_flags_set(furi_thread_get_id(uart->rx_thread), WorkerEvtStop);
        furi_thread_join(uart->rx_thread);
        furi_thread_free(uart->rx_thread);
    }

    if(uart->rx_stream) {
        furi_stream_buffer_free(uart->rx_stream);
    }

    free(uart);
}

bool uart_terminal_uart_send_version_command(Uart* uart) {
    if(!uart) {
        FURI_LOG_E(TAG, "UART_ERROR: Invalid UART context.");
        return false;
    }

    const char* version_command = "VERSION\n";
    uart_terminal_uart_tx(uart, (uint8_t*)version_command, strlen(version_command));

    // Wait for the response
    uint32_t events = furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, 2000);
    if(events & WorkerEvtRxDone) {
        // Split the response at the newline character
        char* newline_pos = strchr(uart->last_response, '\n');
        if(newline_pos) {
            *newline_pos = '\0'; // Terminate the string at the newline character
        }

        char version_check[50];
        snprintf(version_check, sizeof(version_check), "VERSION: %s", MIN_BOARD_VERSION);
        if(strstr(uart->last_response, version_check)) {
            return true;
        }
    }
    FURI_LOG_E(TAG, "UART_ERROR: No response received from the board.");
    return false;
}

bool uart_terminal_uart_check_status(Uart* uart) {
    const char* status_command = "WIFI_STATUS\n";
    uart_terminal_uart_tx(uart, (uint8_t*)status_command, strlen(status_command));

    // Wait for the response
    uint32_t events = furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, 2000);
    if(events & WorkerEvtRxDone) {
        // Split the response at the newline character
        char* newline_pos = strchr(uart->last_response, '\n');
        if(newline_pos) {
            *newline_pos = '\0'; // Terminate the string at the newline character
        }

        // print the response

        FURI_LOG_T(TAG, "Response: %s", uart->last_response);

        if(strstr(uart->last_response, "WIFI_STATUS: CONNECTED")) {
            return true;
        } else if(strstr(uart->last_response, "WIFI_STATUS: DISCONNECTED")) {
            return false;
        }
    }
    FURI_LOG_E(TAG, "UART_ERROR: No response received from the board.");
    return false;
}

bool activeWifiCommand(Uart* uart, const char* argument) {
    UNUSED(argument);
    FURI_LOG_T(TAG, "WIFI_GET_ACTIVE_SSID");

    // Send the command to get the active SSID
    const char* command_ssid = "WIFI_GET_ACTIVE_SSID\n";
    if(!uart_terminal_uart_tx(uart, (uint8_t*)command_ssid, strlen(command_ssid))) {
        FURI_LOG_E(TAG, "UART_ERROR: Failed to retrieve active SSID.");
        return false;
    }

    // Wait for the response
    uint32_t events = furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, 6000);
    if(events & WorkerEvtRxDone) {
        // LOG the response
        FURI_LOG_T(TAG, "Response: %s", uart->last_response);

        // Check if the response contains the active SSID
        if(strncmp(uart->last_response, "WIFI_GET_ACTIVE_SSID: ", 22) == 0) {
            char* active_ssid = uart->last_response + 22;
            if(strcmp(active_ssid, "Not connected") != 0) {
                // Save the active SSID
                strncpy(uart->app->wifi_list.connected_ssid, active_ssid, MAX_SSID_LENGTH - 1);
                uart->app->wifi_list.connected_ssid[MAX_SSID_LENGTH - 1] =
                    '\0'; // Ensure null-termination
            } else {
                uart->app->wifi_list.connected_ssid[0] = '\0'; // Clear the SSID if not connected
            }
            return true;
        }
    }

    FURI_LOG_E(TAG, "UART_ERROR: Failed to retrieve active SSID.");
    return false;
}

bool listWiFiCommand(Uart* uart, const char* argument) {
    UNUSED(argument);
    FURI_LOG_T(TAG, "WIFI_LIST: <list>");
    // Send the command to the UART
    const char* command = "WIFI_LIST\n";
    if(!uart_terminal_uart_tx(uart, (uint8_t*)command, strlen(command))) {
        FURI_LOG_E(TAG, "UART_ERROR: Failed to retrieve WiFi list.");
        return false;
    }

    // Wait for the response
    uint32_t events = furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, 6000);
    if(events & WorkerEvtRxDone) {
        // LOG the response
        FURI_LOG_T(TAG, "Response: %s", uart->last_response);

        // Directly start parsing the response
        char* ssid_list_start = uart->last_response;

        FURI_LOG_T(TAG, "SSID list starts at: %s", ssid_list_start);

        char* start = ssid_list_start;
        char* end = ssid_list_start;
        int index = 0;

        // Clear the existing wifi_list
        memset(&uart->app->wifi_list, 0, sizeof(AvailableWifiList));

        // Parse the SSIDs and populate the wifi_list
        while(*end != '\0' && index < MAX_WIFI_NETWORKS) {
            if(*end == ',') {
                *end = '\0';

                // Trim leading spaces
                while(*start == ' ') {
                    start++;
                }

                strncpy(uart->app->wifi_list.networks[index].ssid, start, MAX_SSID_LENGTH - 1);
                uart->app->wifi_list.networks[index].ssid[MAX_SSID_LENGTH - 1] =
                    '\0'; // Ensure null-termination
                FURI_LOG_T(TAG, "Parsed SSID: %s", uart->app->wifi_list.networks[index].ssid);
                start = end + 1;
                index++;
            }
            end++;
        }

        // Add the last SSID if there is no trailing comma
        if(*start != '\0' && index < MAX_WIFI_NETWORKS) {
            // Trim leading spaces
            while(*start == ' ') {
                start++;
            }

            strncpy(uart->app->wifi_list.networks[index].ssid, start, MAX_SSID_LENGTH - 1);
            uart->app->wifi_list.networks[index].ssid[MAX_SSID_LENGTH - 1] =
                '\0'; // Ensure null-termination
            FURI_LOG_D(TAG, "Parsed SSID: %s", uart->app->wifi_list.networks[index].ssid);
        }

        // Retrieve the active SSID
        if(!activeWifiCommand(uart, NULL)) {
            FURI_LOG_E(TAG, "UART_ERROR: Failed to retrieve active SSID.");
            return false;
        }

        return true;
    }

    FURI_LOG_E(TAG, "UART_ERROR: Failed to retrieve WiFi list.");
    return false;
}

bool disconnectWiFiCommand(Uart* uart, const char* argument) {
    UNUSED(argument);
    FURI_LOG_T(TAG, "WIFI_DEACTIVATE: Wifi disconnected");
    // Send the command to the UART
    const char* command = "WIFI_DEACTIVATE\n";
    if(uart_terminal_uart_tx(uart, (uint8_t*)command, strlen(command))) {
        // Wait for the response
        uint32_t events = furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, 2000);
        if(events & WorkerEvtRxDone) {
            // Split the response at the newline character
            char* newline_pos = strchr(uart->last_response, '\n');
            if(newline_pos) {
                *newline_pos = '\0'; // Terminate the string at the newline character
            }

            // print the response
            FURI_LOG_E("UART", "Response: %s", uart->last_response);

            if(strstr(uart->last_response, "WIFI_DISCONNECT: Wifi disconnected")) {
                return true;
            }
        }
        FURI_LOG_E("UART", "No response received from the board.");
        return false;
    }
    return false;
}
bool getCommand(Uart* uart, const char* argument) {
    FURI_LOG_D("UART_CMDS", "GET %s", argument);

    char command[256];

    // Build the command
    if(uart->app->display_mode == DISPLAY_GET) {
        snprintf(command, sizeof(command), "GET %s\n", argument);
    } else if(uart->app->display_mode == DISPLAY_GET_STREAM) {
        snprintf(command, sizeof(command), "GET_STREAM %s\n", argument);
    }
    uart->app->full_response = true;

    // Send the command to the UART
    if(!uart_terminal_uart_tx(uart, (uint8_t*)command, strlen(command))) {
        return false;
    }

    // Wait for the response
    uint32_t events = furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, 6000);
    if(events & WorkerEvtRxDone) {
        if(uart->app->full_response) {
            // Check if text_box_store is empty or null
            if(furi_string_size(uart->app->text_box_store) == 0) {
                // Input error message to text_box_store
                furi_string_set_str(
                    uart->app->text_box_store, "GET_ERROR: Failed to send GET request");
            }
        }
    } else {
        FURI_LOG_E("UART", "No response received from the board.");
        return false;
    }

    // Check if there is a response otherwise input error message to text_box_store
    if(furi_string_size(uart->app->text_box_store) == 0) {
        FURI_LOG_E(TAG, "GET_ERROR: Failed to send GET request");
        furi_string_set_str(uart->app->text_box_store, "GET_ERROR: Failed to send GET request");
        return false;
    }

    return true;
}
static void uart_timer_callback(void* context) {
    UNUSED(context);
    // This function is called every second by the timer
    // We don't need to do anything here as the main loop in saveToFileCommand
    // will check the bytes_written value
}

bool saveToFileCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(TAG, "FILE_STREAM: %s\n", argument);
    uart->app->save_to_file = true;
    uart->streaming = false;
    uart->bytes_written = 0;

    char command[256];
    snprintf(command, sizeof(command), "FILE_STREAM %s\n", argument);
    if(!uart_terminal_uart_tx(uart, (uint8_t*)command, strlen(command))) {
        return false;
    }

    uint32_t events = furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, 5000);
    if(!(events & WorkerEvtRxDone)) {
        FURI_LOG_E(TAG, "No initial response received from the board.");
        return false;
    }

    FURI_LOG_D(TAG, "Streaming started");
    uart->streaming = true;

    FuriTimer* check_timer = furi_timer_alloc(uart_timer_callback, FuriTimerTypePeriodic, uart);
    furi_timer_start(check_timer, 1000); // Check every 1 second

    size_t last_bytes_written = 0;
    uint32_t idle_counter = 0;

    while(uart->streaming) {
        furi_delay_ms(1000); // Wait for 1 second

        if(uart->bytes_written > last_bytes_written) {
            // Data is still being written
            last_bytes_written = uart->bytes_written;
            idle_counter = 0;
        } else {
            // No new data written
            idle_counter++;
        }

        if(idle_counter >= 3) { // 3 seconds of no new data
            uart->streaming = false;
        }
    }

    furi_timer_stop(check_timer);
    furi_timer_free(check_timer);

    // Perform file check
    FuriString* full_path = furi_string_alloc();
    furi_string_printf(full_path, "%s/%s", APP_DATA_PATH(""), uart->app->filename);

    bool file_present = storage_file_exists(uart->app->storage, furi_string_get_cstr(full_path));
    bool no_error = !strstr(furi_string_get_cstr(uart->app->text_box_store), "DOWNLOAD_ERROR:");

    if(no_error && file_present) {
        furi_string_set_str(uart->app->text_box_store, "saved to sdcard/app_data/");
        furi_string_cat_str(uart->app->text_box_store, uart->app->filename);
        FURI_LOG_T(TAG, "TEXT BOX: %s", furi_string_get_cstr(uart->app->text_box_store));
        update_download_progress(uart->app, uart->bytes_written);
    } else {
        furi_string_set_str(uart->app->text_box_store, "failed to write file");
    }

    furi_string_free(full_path);
    uart->bytes_written = 0;
    uart->app->save_to_file = false;
    return no_error && file_present;
}

bool postCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(
        TAG,
        "POST: %s\nPayload: <json_payload>\nSTATUS: "
        "<number>\nRESPONSE:\n<response>\nRESPONSE_END",
        argument);
    // Send the command to the UART
    return uart_terminal_uart_tx(uart, (uint8_t*)argument, strlen(argument));
}

bool buildHttpMethodCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(TAG, "HTTP_SET_METHOD: %s", argument);
    // Send the command to the UART
    return uart_terminal_uart_tx(uart, (uint8_t*)argument, strlen(argument));
}

bool buildHttpUrlCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(TAG, "HTTP_URL: %s", argument);
    // Send the command to the UART
    return uart_terminal_uart_tx(uart, (uint8_t*)argument, strlen(argument));
}

bool buildHttpHeaderCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(TAG, "HTTP_ADD_HEADER: %s", argument);
    // Send the command to the UART
    return uart_terminal_uart_tx(uart, (uint8_t*)argument, strlen(argument));
}

bool buildHttpPayloadCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(TAG, "HTTP_SET_PAYLOAD: %s", argument);
    // Send the command to the UART
    return uart_terminal_uart_tx(uart, (uint8_t*)argument, strlen(argument));
}

bool removeHttpHeaderCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(TAG, "HTTP_REMOVE_HEADER: %s", argument);
    // Send the command to the UART
    return uart_terminal_uart_tx(uart, (uint8_t*)argument, strlen(argument));
}

bool resetHttpConfigCommand(Uart* uart, const char* argument) {
    UNUSED(argument);
    FURI_LOG_T(TAG, "HTTP_CONFIG_REST: All configurations reset");
    // Send the command to the UART
    const char* command = "RESET_HTTP_CONFIG\n";
    return uart_terminal_uart_tx(uart, (uint8_t*)command, strlen(command));
}

bool buildHttpShowResponseHeadersCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(TAG, "HTTP_BUILDER_SHOW_RESPONSE_HEADERS: %s", argument);
    // Send the command to the UART
    return uart_terminal_uart_tx(uart, (uint8_t*)argument, strlen(argument));
}

bool buildHttpImplementationCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(TAG, "HTTP_SET_IMPLEMENTATION: %s", argument);
    // Send the command to the UART
    return uart_terminal_uart_tx(uart, (uint8_t*)argument, strlen(argument));
}

bool executeHttpCallCommand(Uart* uart, const char* argument) {
    UNUSED(argument);
    FURI_LOG_T("UART_CMDS", "EXECUTE_HTTP_CALL");
    // Send the command to the UART
    const char* command = "EXECUTE_HTTP_CALL\n";
    return uart_terminal_uart_tx(uart, (uint8_t*)command, strlen(command));
}

bool connectCommand(Uart* uart, const char* argument) {
    FURI_LOG_T(
        TAG,
        "WIFI_SSID: <SSID>\nWIFI_PASSWORD: "
        "<password>\nWIFI_CONNECT: Connecting to WiFi...");
    // Send the command to the UART
    if(uart_terminal_uart_tx(uart, (uint8_t*)argument, strlen(argument))) {
        // Wait for the response
        uint32_t events = furi_thread_flags_wait(WorkerEvtRxDone, FuriFlagWaitAny, 5000);
        if(events & WorkerEvtRxDone) {
            // Split the response at the newline character
            char* newline_pos = strchr(uart->last_response, '\n');
            if(newline_pos) {
                *newline_pos = '\0'; // Terminate the string at the newline character
            }

            // print the response
            FURI_LOG_T(TAG, "Response: %s", uart->last_response);

            if(strstr(uart->last_response, "WIFI_SUCCESS: WiFi connected")) {
                return true;
            }
        }
        FURI_LOG_E(TAG, "UART_ERROR: No response received from the board.");
        return false;
    }
    return false;
}
