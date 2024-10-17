
#include "../app.h"
#include "../structs.h"
#include "./main_menu/main_menu.h"
#include <gui/modules/dialog_ex.h>
#include <gui/view.h>

#include "./connect/connect.h"
#include "./connect_favs/connect_favs.h"
#include "./connect_details/connect_details.h"
#include "./text_input/text_input.h"
#include "./setup_dialog/setup_dialog.h"

#include "./get/get.h"

#define TAG "connect_ssid_password"
/** collection of all scene on_enter handlers - in the same order as their enum
 */
void (*const scene_on_enter_handlers[])(void*) = {
    scene_on_enter_setup_dialog,
    scene_on_enter_main_menu,
    scene_on_enter_connect,
    scene_on_enter_connect_details,
    scene_on_enter_text_input,
    scene_on_enter_connect_favs,
    scene_on_enter_get};

/** collection of all scene on event handlers - in the same order as their enum
 */
bool (*const scene_on_event_handlers[])(void*, SceneManagerEvent) = {
    scene_on_event_setup_dialog,
    scene_on_event_main_menu,
    scene_on_event_connect,
    scene_on_event_connect_details,
    scene_on_event_text_input,
    scene_on_event_connect_favs,
    scene_on_event_get};

/** collection of all scene on exit handlers - in the same order as their enum
 */
void (*const scene_on_exit_handlers[])(void*) = {
    scene_on_exit_setup_dialog,
    scene_on_exit_main_menu,
    scene_on_exit_connect,
    scene_on_exit_connect_details,
    scene_on_exit_text_input,
    scene_on_exit_connect_favs,
    scene_on_exit_get};

/** collection of all on_enter, on_event, on_exit handlers */
const SceneManagerHandlers scene_event_handlers = {
    .on_enter_handlers = scene_on_enter_handlers,
    .on_event_handlers = scene_on_event_handlers,
    .on_exit_handlers = scene_on_exit_handlers,
    .scene_num = Count};

/** custom event handler - passes the event to the scene manager */
bool scene_manager_custom_event_callback(void* context, uint32_t custom_event) {
    FURI_LOG_T(TAG, "scene_manager_custom_event_callback");
    furi_assert(context);
    App* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, custom_event);
}

/** navigation event handler - passes the event to the scene manager */
bool scene_manager_navigation_event_callback(void* context) {
    FURI_LOG_T(TAG, "scene_manager_navigation_event_callback");
    App* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

/** initialise the scene manager with all handlers */
void scene_manager_init(App* app) {
    FURI_LOG_T(TAG, "scene_manager_init");
    app->scene_manager = scene_manager_alloc(&scene_event_handlers, app);
}

/** initialise the views, and initialise the view dispatcher with all views */
void view_dispatcher_init(App* app) {
    FURI_LOG_T(TAG, "view_dispatcher_init");
    app->view_dispatcher = view_dispatcher_alloc();

    // allocate each view
    FURI_LOG_D(TAG, "view_dispatcher_init allocating views");
    // Start up and board checks
    app->dialog = dialog_ex_alloc();

    // Main menu
    app->menu = menu_alloc();

    // Connect submenu (list wifi's)
    app->submenu = submenu_alloc();

    // Connect submenu (list wifi's)
    app->submenu_favs = submenu_alloc();

    // Connect details submenu (connect, disconnect, save to csv)
    app->submenu_wifi = submenu_alloc();

    // Text input for ssid and password
    app->text_input = uart_text_input_alloc();
    app->is_custom_tx_string = false;
    app->selected_tx_string = "";

    // Get / Post view
    app->variable_item_list = variable_item_list_alloc();

    // assign callback that pass events from views to the scene manager
    FURI_LOG_D(TAG, "view_dispatcher_init setting callbacks");
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, scene_manager_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, scene_manager_navigation_event_callback);

    // MAIN MENU
    // add views to the dispatcher, indexed by their enum value
    FURI_LOG_D(TAG, "view_dispatcher_init adding view menu");

    view_dispatcher_add_view(
        app->view_dispatcher, AppView_SetupDialog, dialog_ex_get_view(app->dialog));

    view_dispatcher_add_view(app->view_dispatcher, AppView_Menu, menu_get_view(app->menu));

    view_dispatcher_add_view(
        app->view_dispatcher, AppView_Connect, submenu_get_view(app->submenu));

    view_dispatcher_add_view(
        app->view_dispatcher, AppView_Connect_Details, submenu_get_view(app->submenu_wifi));

    view_dispatcher_add_view(
        app->view_dispatcher,
        AppView_Connect_Ssid_Password,
        uart_text_input_get_view(app->text_input));

    view_dispatcher_add_view(
        app->view_dispatcher, AppView_Connect_Favs, submenu_get_view(app->submenu_favs));

    view_dispatcher_add_view(
        app->view_dispatcher, AppView_Get, variable_item_list_get_view(app->variable_item_list));
}
