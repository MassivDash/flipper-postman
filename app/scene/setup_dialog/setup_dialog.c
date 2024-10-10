#include "../../app.h"

#include "../../uart/uart.h"
#include <gui/modules/dialog_ex.h>
#include <gui/view_dispatcher.h>
#include <postmanflipx_icons.h>

#define TAG "tracker_app"
#define WELCOME_DURATION_MS 3000

static void setup_dialog_callback(DialogExResult result, void *context) {
  App *app = context;
  view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

static void setup_dialog_update(App *app, const char *message, const Icon *icon,
                                const char *center_button_text) {
  DialogEx *dialog_ex = app->dialog;

  dialog_ex_set_text(dialog_ex, message, 64, 22, AlignLeft, AlignCenter);
  dialog_ex_set_icon(dialog_ex, 1, 1, icon);
  dialog_ex_set_center_button_text(dialog_ex, center_button_text);
  dialog_ex_set_left_button_text(dialog_ex, "Exit");
  dialog_ex_set_result_callback(dialog_ex, setup_dialog_callback);
  dialog_ex_set_context(dialog_ex, app);
}

static void uart_check_task(App *app) {
  // Update dialog to show checking board state
  FURI_LOG_T(TAG, "Updating dialog: Checking the board ...");
  setup_dialog_update(app, "Checking the board ...", &I_dolphinWait_59x54,
                      NULL);

  // wait 3 seconds
  furi_delay_ms(3000);

  // Check the UART version
  bool version_ok = uart_terminal_uart_send_version_command(app->uart);

  if (version_ok) {
    // Update dialog to show success result
    FURI_LOG_T(TAG, "Updating dialog: Board checked");
    setup_dialog_update(app, "Board checked", &I_DolphinReadingSuccess_59x63,
                        "Continue");
  } else {
    // Update dialog to show failure result
    FURI_LOG_T(TAG, "Updating dialog: No board or wrong version");
    setup_dialog_update(app, "No board or wrong version",
                        &I_DolphinReadingSuccess_59x63, "Retry");
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
  setup_dialog_update(app, "Welcome to Postman", &I_dolphinWait_59x54, NULL);

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