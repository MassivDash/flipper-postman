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
    scene_manager_handle_custom_event(app->scene_manager, AppEvent_SetupDialog);
    break;
  case MenuSelection_Get:
    scene_manager_handle_custom_event(app->scene_manager, AppEvent_MainMenu);
    break;
  }
}

void scene_on_enter_main_menu(void *context) {
  FURI_LOG_T(TAG, "scene_on_enter_main_menu");
  App *app = context;
  menu_reset(app->menu);

  if (app->status == BOARD_CONNECTED_WIFI_ON) {
    menu_add_item(app->menu, "Wifi (Connected)", &A_Connect_14,
                  MenuSelection_Connect, menu_callback_main_menu, app);
    menu_add_item(app->menu, "Get", &A_Get_14, MenuSelection_Get,
                  menu_callback_main_menu, app);
    menu_add_item(app->menu, "Post", &A_Post_14, MenuSelection_Post,
                  menu_callback_main_menu, app);
    menu_add_item(app->menu, "Http Builder", &A_Settings_14,
                  MenuSelection_Build_http, menu_callback_main_menu, app);
    menu_add_item(app->menu, "Download Files", &A_Get_14,
                  MenuSelection_Download, menu_callback_main_menu, app);
    menu_add_item(app->menu, "Open listing port", &A_Listen_14,
                  MenuSelection_Listen, menu_callback_main_menu, app);

  } else {
    menu_add_item(app->menu, "Wifi (Disconnected)", &A_Connect_14,
                  MenuSelection_Connect, menu_callback_main_menu, app);
  }

  menu_add_item(app->menu, "About", &A_About_14, MenuSelection_About,
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
    case AppEvent_SetupDialog:
      scene_manager_next_scene(app->scene_manager, MainMenu);
      consumed = true;
      break;
    case AppEvent_MainMenu:
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