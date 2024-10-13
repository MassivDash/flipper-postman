#include "../../app.h"

void uart_terminal_scene_text_input_callback(void *context) {
  App *app = context;

  view_dispatcher_send_custom_event(app->view_dispatcher,
                                    AppEvent_Connect_Ssid_Password);
}

void scene_on_enter_connect_ssid_password(void *context) {
  App *app = context;

  if (false == app->is_custom_tx_string) {
    // Fill text input with selected string so that user can add to it
    size_t length = strlen(app->selected_tx_string);
    furi_assert(length < UART_TERMINAL_TEXT_INPUT_STORE_SIZE);
    bzero(app->text_input_store, UART_TERMINAL_TEXT_INPUT_STORE_SIZE);
    strncpy(app->text_input_store, app->selected_tx_string, length);

    // Add space - because flipper keyboard currently doesn't have a space
    // app->text_input_store[length] = ' ';
    app->text_input_store[length + 1] = '\0';
    app->is_custom_tx_string = true;
  }

  // Setup view
  UART_TextInput *text_input = app->text_input;
  // Add help message to header
  uart_text_input_set_header_text(text_input, "Enter ssid and password");
  uart_text_input_set_result_callback(
      text_input, uart_terminal_scene_text_input_callback, app,
      app->text_input_store, UART_TERMINAL_TEXT_INPUT_STORE_SIZE, false);

  view_dispatcher_switch_to_view(app->view_dispatcher,
                                 AppView_Connect_Ssid_Password);
}

bool scene_on_event_connect_ssid_password(void *context,
                                          SceneManagerEvent event) {
  App *app = context;
  bool consumed = false;

  if (event.type == SceneManagerEventTypeCustom) {
    if (event.event == AppEvent_Connect_Ssid_Password) {
      // Point to custom string to send
      app->selected_tx_string = app->text_input_store;
      scene_manager_previous_scene(app->scene_manager);
      consumed = true;
    }
  }

  return consumed;
}

void scene_on_exit_connect_ssid_password(void *context) {
  App *app = context;

  uart_text_input_reset(app->text_input);
}