#include "../../app.h"
#include "../../structs.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/variable_item_list.h>

#define TAG "get_app"

typedef enum {
    GetItemHeader,
    GetItemSetUrl,
    GetItemToggleViewSave,
    GetItemAction,
    GetItemCount
} GetItem;

static void get_variable_item_list_callback(void* context, uint32_t index) {
    App* app = context;
    UNUSED(app);

    switch(index) {
    case GetItemSetUrl:
        // Logic to set URL
        // For example, open a text input dialog to set the URL
        break;
    case GetItemToggleViewSave:
        // Toggle between view and save to file
    default:
        break;
    }
}

void scene_on_enter_get(void* context) {
    FURI_LOG_T(TAG, "scene_on_enter_get");
    App* app = context;

    variable_item_list_reset(app->variable_item_list);

    variable_item_list_set_enter_callback(
        app->variable_item_list, get_variable_item_list_callback, app);

    variable_item_list_add(app->variable_item_list, "Set URL", 0, NULL, NULL);
    variable_item_list_add(app->variable_item_list, "Mode", 2, NULL, NULL);
    variable_item_list_add(app->variable_item_list, "Send Request", 0, NULL, NULL);

    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Get);
}

bool scene_on_event_get(void* context, SceneManagerEvent event) {
    FURI_LOG_T(TAG, "scene_on_event_get");
    App* app = context;
    UNUSED(app);
    bool consumed = false;

    switch(event.type) {
    case SceneManagerEventTypeCustom:
        switch(event.event) {
        case AppEvent_Get:
            // Logic to handle setting the URL
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return consumed;
}

void scene_on_exit_get(void* context) {
    FURI_LOG_T(TAG, "scene_on_exit_get");
    App* app = context;

    variable_item_list_reset(app->variable_item_list);
}
