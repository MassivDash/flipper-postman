#ifndef APP_H
#define APP_H

#include "./structs.h"
#include "./uart/uart.h"
#include <gui/modules/dialog_ex.h>
#include <gui/modules/menu.h>
#include <gui/modules/number_input.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>
#include <gui/scene_manager.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>

#include "./modules/uart_text_input/uart_text_input.h"

#define KEY_NAME_SIZE                       22
#define TEXT_STORE_SIZE                     40
#define UART_TERMINAL_TEXT_INPUT_STORE_SIZE (512)

typedef enum {
    BOARD_CONNECTED_WIFI_OFF,
    BOARD_CONNECTED_WIFI_ON,
    BOARD_ERROR
} UartStatus;

typedef struct {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Menu* menu; // Main menu
    Submenu* submenu;
    Submenu* submenu_favs; // Wifi favs
    Submenu* submenu_wifi; // Selected wifi submenu
    FuriTimer* timer;
    DialogEx* dialog; // Setup dialog
    SetupDialogState dialog_state; // State for setup dialog
    Uart* uart; // Uart communications
    uint8_t uart_ch; // Uart channel (USART1)
    UartStatus status; // Uart status
    AvailableWifiList wifi_list; // Wifi struct for operating on wifis
    UART_TextInput* text_input; // Custom text input for passwords and urls
    char text_input_store[UART_TERMINAL_TEXT_INPUT_STORE_SIZE + 1]; // Store for text input
    bool is_custom_tx_string; // Flag for custom text input
    const char* selected_tx_string; // Selected text input string
    File* file; // CSV file for storing wifi credentials
    WifiCredential csv_networks[MAX_WIFI_NETWORKS]; // List of wifi networks from csv
    VariableItemList* variable_item_list; // Variable item list for get view
} App;

#endif // APP_H
