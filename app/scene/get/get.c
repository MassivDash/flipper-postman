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
    furi_assert(app->view_dispatcher);
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

// Dummy function to get the URL
static void get_scene_set_url_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    furi_assert(app);
}

static void get_scene_method_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    furi_assert(app);
    // Toggle between stream and get
    app->get_state->method = !app->get_state->method;
    draw_get_menu(app);
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

    if(!app->get_state) {
        app->get_state = malloc(sizeof(GetState));
        app->get_state->mode = false; // Default mode: Display
        strcpy(app->get_state->url, ""); // Default URL: empty
    }

    // Add items to the variable item list and set their values
    VariableItem* item;

    item = variable_item_list_add(variable_item_list, "Mode", 2, get_scene_mode_callback, app);
    variable_item_set_current_value_text(item, app->get_state->mode ? "Save" : "Display");

    if(!app->get_state->mode) {
        // if not save to file mode, add method
        item = variable_item_list_add(
            variable_item_list, "Method", 2, get_scene_method_callback, app);
        variable_item_set_current_value_text(item, app->get_state->method ? "Stream" : "Get");
    }

    item =
        variable_item_list_add(variable_item_list, "Set URL", 0, get_scene_set_url_callback, app);

    if(strlen(app->get_state->url) > 0) {
        item = variable_item_list_add(
            variable_item_list,
            app->get_state->mode ? "Save to File" : "Send Request",
            0,
            get_scene_action_callback,
            app);
    }
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
        app->get_state->method = false; // Default method: GET
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
            if(strcmp(app->get_state->url, "") == 0) {
                strcpy(app->get_state->url, "https://animechan.io/api/v1/quotes/random");
            }
            app->selected_tx_string = app->get_state->url;
            app->text_input_state = TextInputState_GetUrl;
            scene_manager_next_scene(app->scene_manager, Text_Input);
            break;
        case GetItemToggleViewSave:
            // No logic
            break;
        case GetItemAction:
            // Logic to handle action

            // Depending on the mode, either save to file or send request
            // If mode is save to file, save to file

            if(app->get_state->mode) {
                // Save to file
            } else {
                // If mode is send request, send request

                // If method is GET, send GET request to the URL
                // If method is GET_STREAM, send GET_STREAM request to the URL
                if(app->get_state->method) {
                    app->display_mode = DISPLAY_GET_STREAM;
                } else {
                    app->display_mode = DISPLAY_GET;
                }
                scene_manager_next_scene(app->scene_manager, Display);
            }

            break;
        default:
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
