#include "./connect_details.h"
#include "../../app.h"
#include "../../structs.h"
#include "../../uart/uart.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/submenu.h>

#define TAG "connect_details_app"

void submenu_callback_disconnect_connect(void *context, uint32_t index) {
  UNUSED(index);
  FURI_LOG_T(TAG, "submenu_callback_disconnect_connect");
  App *app = context;

  if (app->status == BOARD_CONNECTED_WIFI_ON) {
    // Disconnect logic here
    FURI_LOG_I(TAG, "Disconnecting from WiFi");
    if (disconnectWiFiCommand(app->uart, NULL)) {

      app->status = BOARD_CONNECTED_WIFI_OFF;
      app->wifi_list.connected_ssid[0] = '\0';
      scene_on_enter_connect_details(context);
    } else {
      FURI_LOG_E(TAG, "Failed to disconnect from WiFi");
    }
  } else {
    // Connect logic here
    FURI_LOG_I(TAG, "Connecting to WiFi");

    // Creating the string for connect cmd, WIFI_CONNECT: <ssid> <password>
    char connect_cmd[CONNECT_CMD_BUFFER_SIZE];

    int ret =
        snprintf(connect_cmd, sizeof(connect_cmd), "WIFI_CONNECT %s %s",
                 app->wifi_list.selected_ssid, app->wifi_list.password_ssid);

    if (ret < 0 || ret >= (int)sizeof(connect_cmd)) {
      // Handle error: the output was truncated or an encoding error occurred
      // You can log an error message or take appropriate action here
      printf("Error: connect_cmd buffer is too small or encoding error "
             "occurred.\n");
    }

    if (connectCommand(app->uart, connect_cmd)) {
      app->status = BOARD_CONNECTED_WIFI_ON;
      strncpy(app->wifi_list.connected_ssid, app->wifi_list.selected_ssid,
              sizeof(app->wifi_list.connected_ssid) - 1);
      app->wifi_list.connected_ssid[sizeof(app->wifi_list.connected_ssid) - 1] =
          '\0'; // Ensure null-termination
      scene_on_enter_connect_details(context);
    } else {
      FURI_LOG_E(TAG, "Failed to connect to WiFi");
    }
  }

  // Refresh the scene
  scene_manager_handle_custom_event(app->scene_manager,
                                    AppEvent_Connect_Details);
}

void submenu_callback_set_password(void *context, uint32_t index) {
  UNUSED(index);
  FURI_LOG_T(TAG, "submenu_callback_set_password");
  App *app = context;
  scene_manager_next_scene(app->scene_manager, Connect_Ssid_Password);
}

void submenu_callback_save_to_csv(void *context, uint32_t index) {
  UNUSED(index);
  FURI_LOG_T(TAG, "submenu_callback_save_to_csv");
  App *app = context;
  UNUSED(app);

  // Logic to save to CSV and mark as default
  FURI_LOG_I(TAG, "Saving WiFi details to CSV and marking as default");
}

void scene_on_enter_connect_details(void *context) {
  FURI_LOG_T(TAG, "scene_on_enter_connect_details");
  App *app = context;

  submenu_reset(app->submenu_wifi);
  char header[64];
  if (app->status == BOARD_CONNECTED_WIFI_ON &&
      strcmp(app->wifi_list.selected_ssid, app->wifi_list.networks[0].ssid) ==
          0) {
    snprintf(header, sizeof(header), "%s (connected)",
             app->wifi_list.selected_ssid);
  } else {
    snprintf(header, sizeof(header), "%s", app->wifi_list.selected_ssid);
  }
  submenu_set_header(app->submenu_wifi, header);

  if (app->status == BOARD_CONNECTED_WIFI_ON) {
    submenu_add_item(app->submenu_wifi, "Disconnect", Details_Disconnect,
                     submenu_callback_disconnect_connect, app);
  } else {
    submenu_add_item(app->submenu_wifi, "Connect", Details_Disconnect,
                     submenu_callback_disconnect_connect, app);
  }
  submenu_add_item(app->submenu_wifi, "Set Password", Details_SetPassword,
                   submenu_callback_set_password, app);
  submenu_add_item(app->submenu_wifi, "Save to flipper", Details_SaveToCsv,
                   submenu_callback_save_to_csv, app);
  submenu_add_item(app->submenu_wifi, "Mark as default", Details_MarkAsDefault,
                   submenu_callback_save_to_csv, app);
  submenu_add_item(app->submenu_wifi, "Forget network", Details_Forget,
                   submenu_callback_save_to_csv, app);
  view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Connect_Details);
}

bool scene_on_event_connect_details(void *context, SceneManagerEvent event) {
  FURI_LOG_T(TAG, "scene_on_event_connect_details");
  App *app = context;
  UNUSED(app);
  bool consumed = false;
  switch (event.type) {
  case SceneManagerEventTypeCustom:
    switch (event.event) {
    case AppView_Connect_Details:
      scene_on_enter_connect_details(context);
      consumed = true;
      break;
    }
    break;

  default:
    consumed = false;
    break;
  }
  return consumed;
}

void scene_on_exit_connect_details(void *context) {
  FURI_LOG_T(TAG, "scene_on_exit_connect_details");
  App *app = context;
  submenu_reset(app->submenu_wifi);
}