#include "../app.h"
#include "../structs.h"
#include "../uart/uart.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/icon_i.h>
#include <gui/modules/menu.h>
#include <gui/modules/popup.h>
#include <gui/modules/submenu.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>

void app_free(App* app) {
    FURI_LOG_T(TAG, "free");
    uart_terminal_uart_free(app->uart);
    scene_manager_free(app->scene_manager);

    // Scene on exit handlers
    view_dispatcher_remove_view(app->view_dispatcher, AppView_SetupDialog);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Menu);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Connect);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Connect_Details);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Connect_Text_Input);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Connect_Favs);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Get);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Display);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Get_Url_List);
    view_dispatcher_free(app->view_dispatcher);

    // Memory for the UI
    menu_free(app->menu);
    submenu_free(app->submenu);
    variable_item_list_free(app->variable_item_list);
    dialog_ex_free(app->dialog);
    widget_free(app->text_box);
    uart_text_input_free(app->text_input);

    // Memory for the CSV
    for(int i = 0; i < MAX_URLS; i++) {
        free(app->url_list[i].url); // Free dynamically allocated memory within each UrlList
    }

    // Memory for the WiFi networks
    for(int i = 0; i < MAX_WIFI_NETWORKS; i++) {
        free(app->csv_networks[i].ssid); // Free dynamically allocated memory for ssid
        free(app->csv_networks[i].password); // Free dynamically allocated memory for password
    }

    furi_string_free(app->text_box_store);
    furi_timer_free(app->timer);

    furi_record_close(RECORD_STORAGE);
    free(app);
}
