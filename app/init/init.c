#include "../app.h"
#include "../csv/csv.h"
#include "../scene/scene.h"
#include "../uart/uart.h"

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

    // Initialize the CSV file
    // Sync the CSV networks to app->csv_networks
    if(!init_csv(app->file, app->csv_networks)) {
        FURI_LOG_E(TAG, "Failed to initialize CSV");
        free(app);
        return NULL;
    }

    // Initialize the scene manager (GUI)
    scene_manager_init(app);
    view_dispatcher_init(app);
    FURI_LOG_T(TAG, "Successful initializing postman flipper app");
    return app;
}
