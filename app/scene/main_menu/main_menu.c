#include "../../app.h"
#include "../../structs.h"
#include <datetime/datetime.h>
#include <furi.h>

#define TAG "tracker_app"
#include <postmanflipx_icons.h>

/** main menu callback - sends a custom event to the scene manager based on the
 * menu selection */
void menu_callback_main_menu(void *context, uint32_t index) {
  FURI_LOG_T(TAG, "menu_callback_main_menu");
  App *app = context;
  switch (index) {
  case MenuSelection_Connect:
    scene_manager_handle_custom_event(app->scene_manager, AppEvent_Connect);
    break;
  case MenuSelection_Get:
    scene_manager_handle_custom_event(app->scene_manager, AppEvent_Get);
    break;
  }
}

void scene_on_enter_main_menu(void *context) {
  FURI_LOG_T(TAG, "scene_on_enter_main_menu");
  App *app = context;
  menu_reset(app->menu);

  menu_add_item(app->menu, "Connect", NULL, MenuSelection_Connect,
                menu_callback_main_menu, app);
  menu_add_item(app->menu, "GET", &A_ViewTasks_14, MenuSelection_Get,
                menu_callback_main_menu, app);

  view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Menu);
}

/** main menu event handler - switches scene based on the event */
bool scene_on_event_main_menu(void *context, SceneManagerEvent event) {
  FURI_LOG_T(TAG, "scene_on_event_main_menu");
  App *app = context;
  bool consumed = false;
  switch (event.type) {
  case SceneManagerEventTypeCustom:
    switch (event.event) {
    case AppEvent_Connect:
      scene_manager_next_scene(app->scene_manager, MainMenu);
      consumed = true;
      break;
    case AppEvent_Get:
      scene_manager_next_scene(app->scene_manager, MainMenu);
      consumed = true;
      break;
    }
    break;

  default: // eg. SceneManagerEventTypeBack, SceneManagerEventTypeTick
    consumed = false;
    break;
  }
  return consumed;
}

void scene_on_exit_main_menu(void *context) {
  FURI_LOG_T(TAG, "scene_on_exit_main_menu");
  App *app = context;
  menu_reset(app->menu);
}