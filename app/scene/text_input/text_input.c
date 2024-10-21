#include "../../app.h"

void uart_terminal_scene_text_input_callback(void* context) {
    App* app = context;

    switch(app->text_input_state) {
    case TextInputState_GetUrl:
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_Get);
        break;
    case TextInputState_WifiPassword:
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_Input_Text);
        break;
    // case TextInputState_PostUrl:
    //     view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_Post);
    //     break;
    case TextInputState_Filename:
        view_dispatcher_send_custom_event(app->view_dispatcher, AppEvent_Set_Filename);
        break;
    case TextInputState_Payload:
        // Handle payload input completion
        break;
    case TextInputState_Message:
        // Handle message input completion
        break;
    default:
        break;
    }
}

void scene_on_enter_text_input(void* context) {
    App* app = context;

    if(false == app->is_custom_tx_string) {
        // Fill text input with selected string so that user can add to it
        size_t length = strlen(app->selected_tx_string);
        furi_assert(length < UART_TERMINAL_TEXT_INPUT_STORE_SIZE);
        strncpy(app->text_input_store, app->selected_tx_string, length);

        // Add space - because flipper keyboard currently doesn't have a space
        // app->text_input_store[length] = ' ';
        app->text_input_store[length + 1] = '\0';
        app->is_custom_tx_string = true;
    }

    // Setup view
    UART_TextInput* text_input = app->text_input;
    // Add help message to header
    const char* header_text = "";
    switch(app->text_input_state) {
    case TextInputState_GetUrl:
        header_text = "Enter URL";
        break;
    case TextInputState_WifiPassword:
        header_text = "Enter Password";
        break;
    case TextInputState_PostUrl:
        header_text = "Enter Post URL";
        break;
    case TextInputState_Filename:
        header_text = "Enter Filename";
        break;
    case TextInputState_Payload:
        header_text = "Enter Payload";
        break;
    case TextInputState_Message:
        header_text = "Enter Message";
        break;
    default:
        break;
    }
    uart_text_input_set_header_text(text_input, header_text);
    uart_text_input_set_result_callback(
        text_input,
        uart_terminal_scene_text_input_callback,
        app,
        app->text_input_store,
        UART_TERMINAL_TEXT_INPUT_STORE_SIZE,
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Connect_Text_Input);
}

bool scene_on_event_text_input(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(app->text_input_state) {
        case TextInputState_WifiPassword:
            if(event.event == AppEvent_Input_Text) {
                // Handle SSID and password input completion
                app->selected_tx_string = app->text_input_store;
                strncpy(
                    app->wifi_list.password_ssid,
                    app->text_input_store,
                    sizeof(app->wifi_list.password_ssid) - 1);
                app->wifi_list.password_ssid[sizeof(app->wifi_list.password_ssid) - 1] =
                    '\0'; // Ensure null-termination
                consumed = true;
                scene_manager_previous_scene(app->scene_manager);
            }
            break;
        case TextInputState_GetUrl:
            if(event.event == AppEvent_Get) {
                // Handle URL input completion
                strncpy(
                    app->get_state->url, app->text_input_store, sizeof(app->get_state->url) - 1);
                app->get_state->url[sizeof(app->get_state->url) - 1] =
                    '\0'; // Ensure null-termination
                scene_manager_previous_scene(app->scene_manager);
                consumed = true;
            }
            break;
        case TextInputState_Filename:
            if(event.event == AppEvent_Set_Filename) {
                // Handle Filename input completion
                strncpy(app->filename, app->text_input_store, sizeof(app->filename) - 1);
                app->filename[sizeof(app->filename) - 1] = '\0'; // Ensure null-termination
                app->display_mode = DISPLAY_DOWNLOAD;
                consumed = true;
                scene_manager_next_scene(app->scene_manager, Display);
            }
            break;
        default:
            break;
        }
    }

    return consumed;
}

void scene_on_exit_text_input(void* context) {
    App* app = context;
    app->is_custom_tx_string = false;
    app->selected_tx_string = "";
    uart_text_input_reset(app->text_input);
}
