
#include "../../app.h"
#include "../../structs.h"
#include "../../uart/uart.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/submenu.h>

#define TAG "connect_app"

void submenu_callback_select_wifi(void *context, uint32_t index) {
  FURI_LOG_T(TAG, "submenu_callback_select_wifi");
  furi_assert(context);
  App *app = context;

  if (index < MAX_WIFI_NETWORKS &&
      app->wifi_list.networks[index].ssid[0] != '\0') {
    // Set the selected network
    app->wifi_list.selected_network = index;

    // Handle the custom event
    scene_manager_handle_custom_event(app->scene_manager, AppEvent_Connect);
  } else {
    FURI_LOG_W(TAG, "Invalid WiFi index: %lu", (unsigned long)index);
  }
}

void submenu_callback_no_wifi(void *context, uint32_t index) {
  FURI_LOG_T(TAG, "submenu_callback_no_wifi");
  furi_assert(index);
  App *app = context;
  scene_manager_search_and_switch_to_previous_scene(app->scene_manager,
                                                    MainMenu);
}

void scene_on_enter_connect(void *context) {
  FURI_LOG_T(TAG, "scene_on_enter_connect");
  App *app = context;

  submenu_reset(app->submenu);
  submenu_set_header(app->submenu, "Available WiFi's");

  // Add a dummy menu item indicating that the search is in progress
  submenu_add_item(app->submenu, "Searching ...", 0, NULL, NULL);

  view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Connect);

  // Run the listWiFiCommand to get the available WiFi networks
  if (!listWiFiCommand(app->uart, NULL)) {
    FURI_LOG_E(TAG, "Failed to list WiFi networks");
    submenu_reset(app->submenu);
    submenu_add_item(app->submenu, "Failed to list WiFi networks", 0,
                     submenu_callback_no_wifi, app);
    return;
  }

  // Clear the submenu and add the actual WiFi networks

  if (app->wifi_list.networks[0].ssid[0] != '\0') {
    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Available WiFi's");
    // Add WiFi networks to submenu
    for (size_t i = 0;
         i < MAX_WIFI_NETWORKS && app->wifi_list.networks[i].ssid[0] != '\0';
         i++) {
      submenu_add_item(app->submenu, app->wifi_list.networks[i].ssid, i,
                       submenu_callback_select_wifi, app);
    }
  } else {
    submenu_add_item(app->submenu, "No WiFi networks found", 0,
                     submenu_callback_no_wifi, app);
  }
}

bool scene_on_event_connect(void *context, SceneManagerEvent event) {
  FURI_LOG_T(TAG, "scene_on_event_connect");
  App *app = context;
  bool consumed = false;
  switch (event.type) {
  case SceneManagerEventTypeCustom:
    switch (event.event) {
    case AppEvent_Connect:
      // Add log before switching to Connect
      FURI_LOG_I(TAG, "Switching to Connect");
      scene_manager_next_scene(app->scene_manager, Connect);
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

void scene_on_exit_connect(void *context) {
  FURI_LOG_T(TAG, "scene_on_exit_connect");
  App *app = context;
  for (size_t i = 0;
       i < MAX_WIFI_NETWORKS && app->wifi_list.networks[i].ssid[0] != '\0';
       i++) {
    FURI_LOG_T(TAG, "WiFi %lu: SSID: %s", (unsigned long)i,
               app->wifi_list.networks[i].ssid);
  }

  submenu_reset(app->submenu);
}
