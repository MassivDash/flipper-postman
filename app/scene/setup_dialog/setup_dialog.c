#include "../../app.h"

#include "../../uart/uart.h"
#include "../../version.h"
#include <gui/modules/dialog_ex.h>
#include <gui/view_dispatcher.h>
#include <postmanflipx_icons.h>

#define TAG "Postman_SetupDialog"
#define WELCOME_DURATION_MS 3000

static void setup_dialog_callback(DialogExResult result, void *context) {
  App *app = context;
  view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

static void setup_dialog_update(App *app, const char *message, const Icon *icon,
                                const char *center_button_text, int text_x,
                                int text_y, Align text_align_h,
                                Align text_align_v) {
  DialogEx *dialog_ex = app->dialog;

  // Set the text with specified position and alignment
  dialog_ex_set_text(dialog_ex, message, text_x, text_y, text_align_h,
                     text_align_v);
  dialog_ex_set_icon(dialog_ex, 1, 1, icon);

  // Set the center button text only if it is not NULL
  if (center_button_text) {
    dialog_ex_set_center_button_text(dialog_ex, center_button_text);
  }

  dialog_ex_set_left_button_text(dialog_ex, "Exit");
  dialog_ex_set_result_callback(dialog_ex, setup_dialog_callback);
  dialog_ex_set_context(dialog_ex, app);
}

static void uart_check_task(App *app) {
  FURI_LOG_T(TAG, "Updating dialog: Checking the board ...");

  bool version_ok = uart_terminal_uart_send_version_command(app->uart);

  // Update the dialog with the current message
  setup_dialog_update(app, "esp32 check", &I_WifiCheck_128_64, NULL, 57, 58,
                      AlignLeft, AlignCenter);

  if (version_ok) {
    // Update dialog to show success result
    FURI_LOG_T(TAG, "Updating dialog: Board checked");
    setup_dialog_update(app, "Board checked", &I_esp32Success, "Continue", 53,
                        20, AlignLeft, AlignCenter);
  } else {
    // Update dialog to show failure result
    FURI_LOG_T(TAG, "Updating dialog: No board or wrong version");
    setup_dialog_update(app, "Oh no ! No board or missing firmware.", &I_error,
                        "Retry", 60, 25, AlignLeft, AlignCenter);
  }
}

static void setup_dialog_timer_callback(void *context) {
  App *app = context;

  switch (app->dialog_state) {
  case SetupDialogStateWelcome:
    // Move to the next state
    app->dialog_state = SetupDialogStateChecking;
    // Update dialog to show checking board state and start UART check task
    uart_check_task(app);
    break;
  case SetupDialogStateChecking:
  case SetupDialogStateResult:
    // No action needed
    break;
  }
}

void scene_on_enter_setup_dialog(void *context) {
  FURI_LOG_T(TAG, "scene_on_enter_setup_dialog");
  App *app = context;

  // Initialize the dialog state
  app->dialog_state = SetupDialogStateWelcome;

  // Initialize the timer
  app->timer =
      furi_timer_alloc(setup_dialog_timer_callback, FuriTimerTypeOnce, app);

  // Update dialog to show welcome message
  char welcome_message[64];
  snprintf(welcome_message, sizeof(welcome_message), "v%s", VERSION);
  setup_dialog_update(app, welcome_message, &I_Postman_128_64, NULL, 73, 40,
                      AlignLeft, AlignCenter);

  // Start the timer for the welcome message
  furi_timer_start(app->timer, WELCOME_DURATION_MS);

  // Switch to the dialog view
  view_dispatcher_switch_to_view(app->view_dispatcher, AppView_SetupDialog);
}

bool scene_on_event_setup_dialog(void *context, SceneManagerEvent event) {
  App *app = context;
  bool consumed = false;

  if (event.type == SceneManagerEventTypeCustom) {
    switch (event.event) {
    case DialogExResultCenter:
      scene_manager_next_scene(app->scene_manager, MainMenu);
      consumed = true;
      break;
    default:
      consumed = false;
      break;
    }
  }

  return consumed;
}

void scene_on_exit_setup_dialog(void *context) {
  FURI_LOG_T(TAG, "scene_on_exit_setup_dialog");
  App *app = context;

  dialog_ex_reset(app->dialog);

  // Stop and free the timer
  furi_timer_stop(app->timer);
}