#include "../../app.h"
#include "../../structs.h"
#include "./build_http_call.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/variable_item_list.h>

static void build_http_call_select_callback(void* context, uint32_t index) {
    App* app = context;
    furi_assert(app);
    furi_assert(app->view_dispatcher);

    FURI_LOG_D(TAG, "build_http_call_select_callback: %ld", index);

    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void build_http_call_mode_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    furi_assert(app);
    app->build_http_state->mode = !app->build_http_state->mode;
    draw_build_http_call_menu(app);
}

static void build_http_call_method_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    furi_assert(app);
    app->build_http_state->http_method = (app->build_http_state->http_method + 1) % 6;
    draw_build_http_call_menu(app);
}

static void build_http_call_show_headers_callback(VariableItem* item) {
    App* app = variable_item_get_context(item);
    furi_assert(app);
    app->build_http_state->show_response_headers = !app->build_http_state->show_response_headers;
    draw_build_http_call_menu(app);
}

void draw_build_http_call_menu(App* app) {
    furi_assert(app);
    furi_assert(app->variable_item_list);
    VariableItemList* variable_item_list = app->variable_item_list;

    variable_item_list_reset(variable_item_list);

    VariableItem* item;

    const char* mode_names[] = {"Display", "Save"};
    item =
        variable_item_list_add(variable_item_list, "Mode", 2, build_http_call_mode_callback, app);
    variable_item_set_current_value_text(item, mode_names[app->build_http_state->mode]);

    const char* method_names[] = {"HEAD", "GET", "POST", "PATCH", "PUT", "DELETE"};
    item = variable_item_list_add(
        variable_item_list, "HTTP Method", 6, build_http_call_method_callback, app);
    variable_item_set_current_value_text(item, method_names[app->build_http_state->http_method]);

    item = variable_item_list_add(variable_item_list, "Set URL", 0, NULL, app);

    item = variable_item_list_add(variable_item_list, "Set Headers", 0, NULL, app);

    item = variable_item_list_add(
        variable_item_list, "Show Response Headers", 2, build_http_call_show_headers_callback, app);
    variable_item_set_current_value_text(
        item, app->build_http_state->show_response_headers ? "On" : "Off");

    item = variable_item_list_add(variable_item_list, "Set Payload", 0, NULL, app);

    item = variable_item_list_add(variable_item_list, "Send Request", 0, NULL, app);
}

void scene_on_enter_build_http_call(void* context) {
    App* app = context;
    furi_assert(app);
    furi_assert(app->variable_item_list);

    if(!app->build_http_state) {
        app->build_http_state = malloc(sizeof(BuildHttpState));
        app->build_http_state->mode = false;
        app->build_http_state->http_method = GET;
        app->build_http_state->show_response_headers = false;
        strcpy(app->build_http_state->url, "");
        app->build_http_state->payload = furi_string_alloc();
    }

    variable_item_list_set_enter_callback(
        app->variable_item_list, build_http_call_select_callback, app);
    draw_build_http_call_menu(app);
    variable_item_list_set_selected_item(app->variable_item_list, 0);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_BuildHttpCall);
}

bool scene_on_event_build_http_call(void* context, SceneManagerEvent event) {
    App* app = context;
    furi_assert(app);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case 3: // Set URL
            app->selected_tx_string = app->build_http_state->url;
            app->text_input_state = TextInputState_BuildHttpUrl;
            scene_manager_next_scene(app->scene_manager, Text_Input);
            consumed = true;
            break;
        case 4: // Set Headers
            scene_manager_next_scene(app->scene_manager, Build_Http_Headers);
            consumed = true;
            break;
        case 6: // Set Payload
            app->selected_tx_string = furi_string_get_cstr(app->build_http_state->payload);
            app->text_input_state = TextInputState_BuildHttpPayload;
            scene_manager_next_scene(app->scene_manager, Text_Input);
            consumed = true;
            break;
        case 7: // Send Request
            // Implement the logic to send the HTTP request
            // This could involve switching to a new scene or calling a function to handle the request
            consumed = true;
            break;
        default:
            consumed = false;
            break;
        }
    }

    return consumed;
}

void scene_on_exit_build_http_call(void* context) {
    App* app = context;
    furi_assert(app);
    variable_item_list_reset(app->variable_item_list);
}
