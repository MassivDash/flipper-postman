
#ifndef STRUCTS_H
#define STRUCTS_H

typedef enum {
  SetUpDialog,
  MainMenu,
  Connect,
  Connect_Details,
  Connect_Ssid_Password,
  // Get,
  // Post,
  // Build_http,
  // Download,
  // Listen,
  // About,
  Count
} AppScene;

typedef enum {
  AppView_SetupDialog,
  AppView_Menu,
  AppView_Connect,
  AppView_Connect_Details,
  AppView_Connect_Ssid_Password,
  // AppView_Get,
  // AppView_Post,
  // AppView_Build_http,
  // AppView_Download,
  // AppView_Listen,
  // AppView_About,
} AppView;

/** all custom events */
typedef enum {
  AppEvent_SetupDialog,
  AppEvent_MainMenu,
  AppEvent_Connect,
  AppEvent_Connect_Details,
  AppEvent_Connect_Ssid_Password,
  // AppEvent_Get,
  // AppEvent_Post,
  // AppEvent_Build_http,
  // AppEvent_Download,
  // AppEvent_Listen,
  // AppEvent_About,
} AppEvent;

/* main menu scene */

/** indices for menu items */
typedef enum {
  MenuSelection_Connect,
  MenuSelection_Get,
  MenuSelection_Post,
  MenuSelection_Build_http,
  MenuSelection_Download,
  MenuSelection_Listen,
  MenuSelection_About,
} AppMenuSelection;

/** Submenu for get view */
typedef enum {
  Get_SetUrl,
  Get_Execute,
  Get_Stream_Execute,
} GetActionMenu;

// Event Flags for UART Worker Thread

typedef enum {
  SetupDialogStateWelcome,
  SetupDialogStateChecking,
  SetupDialogStateResult
} SetupDialogState;

typedef enum {
  WorkerEvtStop = (1 << 0),
  WorkerEvtRxDone = (1 << 1),
} WorkerEvtFlags;

typedef struct {
  const char *name;
  const char *description;
  void (*execute)(const char *argument);
} Command;

#define RX_BUF_SIZE (4096)
#define MAX_WIFI_NETWORKS 20
#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64
#define CONNECT_CMD_BUFFER_SIZE                                                \
  (16 + MAX_SSID_LENGTH + 1 + MAX_PASSWORD_LENGTH +                            \
   1) // "WIFI_CONNECT: " + ssid + " " + password + null terminator

typedef struct {
  char ssid[MAX_SSID_LENGTH];
} WifiNetwork;

typedef struct {
  WifiNetwork networks[MAX_WIFI_NETWORKS];
  char selected_ssid[MAX_SSID_LENGTH];
  char connected_ssid[MAX_SSID_LENGTH];
  char password_ssid[MAX_PASSWORD_LENGTH];
} AvailableWifiList;

typedef enum {
  Details_Disconnect,
  Details_Connect,
  Details_SetPassword,
  Details_SaveToCsv,
  Details_MarkAsDefault,
  Details_Forget
} Connect_DetailsActionMenu;

#endif // STRUCTS_H
