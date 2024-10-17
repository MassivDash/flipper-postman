#include "../../app.h"
#include "../../structs.h"
#include "get.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/variable_item_list.h>

#define TAG "get_app"

static void get_scene_select_callback(void* context, uint32_t index) {
    App* app = context;
    furi_assert(app);
    furi_assert(index);
}

static void get_scene_set_url_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    furi_assert(app);
    // Logic to set URL
    // For example, open a text input dialog to set the URL
    // Assuming the URL is set in app->get_state->url
}

static void get_scene_mode_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    furi_assert(app);
    // Toggle between view and save to file
    app->get_state->mode = !app->get_state->mode;
    draw_get_menu(app);
}

static void get_scene_action_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    furi_assert(app);
    if(app->get_state->mode) {
        // Logic to save to file
    } else {
        // Logic to send request
    }
}

void draw_get_menu(App* app) {
    furi_assert(app);
    furi_assert(app->variable_item_list);
    VariableItemList* variable_item_list = app->variable_item_list;

    variable_item_list_reset(variable_item_list);

    // Add items to the variable item list and set their values
    VariableItem* item;

    item = variable_item_list_add(variable_item_list, "Mode", 2, get_scene_mode_callback, app);

    variable_item_set_current_value_text(item, app->get_state->mode ? "Save" : "Display");

    item =
        variable_item_list_add(variable_item_list, "Set URL", 0, get_scene_set_url_callback, app);

    item = variable_item_list_add(
        variable_item_list,
        app->get_state->mode ? "Save to File" : "Send Request",
        0,
        get_scene_action_callback,
        app);
}

void scene_on_enter_get(void* context) {
    FURI_LOG_T(TAG, "scene_on_enter_get");
    App* app = context;
    furi_assert(app);
    furi_assert(app->variable_item_list);
    VariableItemList* variable_item_list = app->variable_item_list;

    // Initialize get_state if not already initialized
    if(!app->get_state) {
        app->get_state = malloc(sizeof(GetState));
        app->get_state->mode = false; // Default mode: Display
        strcpy(app->get_state->url, ""); // Default URL: empty
    }

    variable_item_list_set_enter_callback(variable_item_list, get_scene_select_callback, app);
    draw_get_menu(app);
    variable_item_list_set_selected_item(variable_item_list, 0);

    furi_assert(app->view_dispatcher);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Get);
}

bool scene_on_event_get(void* context, SceneManagerEvent event) {
    FURI_LOG_T(TAG, "scene_on_event_get");
    App* app = context;
    furi_assert(app);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        switch(event.event) {
        case GetItemSetUrl:
            // Logic to handle setting the URL
            break;
        case GetItemToggleViewSave:
            // Logic to handle mode change
            break;
        case GetItemAction:
            // Logic to handle action
            break;
        default:
            furi_crash("Unknown key type");
            break;
        }
    }

    return consumed;
}

void scene_on_exit_get(void* context) {
    FURI_LOG_T(TAG, "scene_on_exit_get");
    App* app = context;
    furi_assert(app);
    furi_assert(app->variable_item_list);
    variable_item_list_reset(app->variable_item_list);
}
