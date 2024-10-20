#include "../app.h"
#include "../scene/scene.h"
#include "../uart/uart.h"
#include "app/csv/csv_wifi/csv_wifi.h"
#include "app/csv/csv_get_url/csv_get_url.h"

App* init() {
    FURI_LOG_T(TAG, "Initializing postman flipper app");

    // Allocate memory for the app
    App* app = malloc(sizeof(App));
    if(!app) {
        FURI_LOG_E(TAG, "Failed to allocate memory for App");
        return NULL;
    }

    // Initialize the UART
    // This is the main protocol for communicating with the board
    app->uart = uart_terminal_uart_init(app);
    if(!app->uart) {
        FURI_LOG_E(TAG, "Failed to initialize UART");
        free(app);
        return NULL;
    }

    app->storage = furi_record_open(RECORD_STORAGE);
    // Initialize the CSV file for wifi files
    // Sync the CSV networks to app->csv_networks
    init_csv_wifi(app);

    // Initialize the CSV file for URLs
    init_csv_get_url(app);
    // Initialize the text box for displaying UART responses
    app->text_box_store = furi_string_alloc();

    // Initialize the scene manager (GUI)
    scene_manager_init(app);
    view_dispatcher_init(app);
    FURI_LOG_T(TAG, "Successful initializing postman flipper app");
    return app;
}
