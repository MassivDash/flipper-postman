#include "../app.h"
#include "../structs.h"
#include "../uart/uart.h"
#include <furi.h>
#include <gui/gui.h>
#include <gui/icon_i.h>
#include <gui/modules/menu.h>
#include <gui/modules/popup.h>
#include <gui/modules/submenu.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>

#define TAG "tracker_app"

void app_free(App *app) {
  FURI_LOG_T(TAG, "free");
  uart_terminal_uart_free(app->uart);
  scene_manager_free(app->scene_manager);
  view_dispatcher_remove_view(app->view_dispatcher, AppView_SetupDialog);
  view_dispatcher_remove_view(app->view_dispatcher, AppView_Menu);
  view_dispatcher_free(app->view_dispatcher);
  menu_free(app->menu);
  dialog_ex_free(app->dialog);
  furi_timer_free(app->timer);
  furi_record_close(RECORD_STORAGE);
  free(app);
}