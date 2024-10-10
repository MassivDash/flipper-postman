#ifndef SETUP_DIALOG_H
#define SETUP_DIALOG_H

#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>

#define TAG "tracker_app"

// SetupDialogModel struct
typedef struct {
  char message[64];
  bool uart_connected;
  bool board_checked;
  bool version_checked;
  bool show_start_button;
} SetupDialogModel;

// Function declarations
bool setup_dialog_init(View *view);
void setup_dialog_free(View *view);
void scene_on_enter_setup_dialog(void *context);
bool scene_on_event_setup_dialog(void *context, SceneManagerEvent event);
void scene_on_exit_setup_dialog(void *context);

#endif // SETUP_DIALOG_H