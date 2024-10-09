#include "../app.h"
#include "../structs.h"
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
  scene_manager_free(app->scene_manager);
  view_dispatcher_remove_view(app->view_dispatcher, AppView_Menu);

  view_dispatcher_free(app->view_dispatcher);
  menu_free(app->menu);
  view_free(app->view);
  view_free(app->view);
  submenu_free(app->submenu);
  text_input_free(app->text_input);
  variable_item_list_free(app->variable_item_list);
  dialog_ex_free(app->dialog);
  furi_timer_free(app->timer);
  furi_record_close(RECORD_STORAGE);
  storage_file_free(app->file);
  free(app);
}