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

#define TAG "tracker_app"

void app_free(App* app) {
    FURI_LOG_T(TAG, "free");
    uart_terminal_uart_free(app->uart);
    scene_manager_free(app->scene_manager);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_SetupDialog);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Menu);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Connect);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Connect_Details);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Connect_Text_Input);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Connect_Favs);
    view_dispatcher_remove_view(app->view_dispatcher, AppView_Get);
    view_dispatcher_free(app->view_dispatcher);
    menu_free(app->menu);
    submenu_free(app->submenu);
    submenu_free(app->submenu_favs);
    submenu_free(app->submenu_wifi);
    variable_item_list_free(app->variable_item_list);
    dialog_ex_free(app->dialog);
    uart_text_input_free(app->text_input);
    furi_timer_free(app->timer);
    furi_record_close(RECORD_STORAGE);
    free(app);
}
