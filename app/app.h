#ifndef APP_H
#define APP_H

#include "./structs.h" // Structs
#include "./uart/uart.h" // Uart Communications

//SCENE MANAGER and VIEW
#include <gui/scene_manager.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>

// SCENES
#include <gui/modules/dialog_ex.h> // Setup dialog
#include <gui/modules/menu.h> // Main menu
#include <gui/modules/submenu.h> // Subemenus, Connect, Connect_Favs, Connect_Details
#include <gui/modules/text_box.h> // Text box for displaying Uart responses
#include <gui/modules/variable_item_list.h> // Variable item list for GET view
#include "./modules/uart_text_input/uart_text_input.h" // Custom text input for passwords and urls
#include <gui/modules/widget.h> // As TEXT BOX
// CSV STORAGE
#include <storage/storage.h>

#define KEY_NAME_SIZE                       22
#define TEXT_STORE_SIZE                     128
#define UART_TERMINAL_TEXT_INPUT_STORE_SIZE (512)
#define MAX_URLS                            50
// TAG for logging
#define TAG                                 "FLIPPER_POSTMAN"

// After 7k the widget scroll started to be buggy, were not able to scroll to the end
#define DISPLAY_STORE_SIZE (1024 * 7 - 100) // this seems to be the biggest stable size

typedef struct {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Menu* menu; // Main menu
    Submenu* submenu;
    Submenu* submenu_favs; // Wifi favs
    Submenu* submenu_wifi; // Selected wifi submenu

    // START UP SCENE
    FuriTimer* timer;
    DialogEx* dialog; // Setup dialog
    SetupDialogState dialog_state; // State for setup dialog

    // UART COMMUNICATIONS
    Uart* uart; // Uart communications
    uint8_t uart_ch; // Uart channel (USART1)
    UartStatus status; // Uart status

    // TEXT INPUT
    UART_TextInput* text_input; // Custom text input for passwords and urls
    char text_input_store[UART_TERMINAL_TEXT_INPUT_STORE_SIZE + 1]; // Store for text input
    bool is_custom_tx_string; // Flag for custom text input
    const char* selected_tx_string; // Selected text input string
    TextInputState text_input_state; // Current text input state

    // WIFI CSV LIST
    Storage* storage; // Storage for CSV files
    AvailableWifiList wifi_list; // This holds the board wifi scan results
    WifiCredential csv_networks[MAX_WIFI_NETWORKS]; // List of wifi networks from csv
    File* file_wifi; // CSV file for storing wifi credentials

    // GET VIEW
    VariableItemList* variable_item_list; // Variable item list for get view
    GetState* get_state; // Get state for get view
    UrlList url_list[MAX_URLS]; // List of get urls
    File* file_get_url; // CSV file for storing get urls

    // DISPLAY VIEW
    Widget* text_box; // Text box for displaying uart responses
    FuriString* text_box_store; // Store for displaying uart responses
    bool full_response; // Flag for attaching full uart response otherwise last line
    DisplayMode display_mode; // Display mode for display view

} App;

#endif // APP_H
