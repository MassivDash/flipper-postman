
#ifndef STRUCTS_H
#define STRUCTS_H

typedef enum {
  SetUpDialog,
  MainMenu,
  Connect,
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

typedef struct {
  char ssid[MAX_SSID_LENGTH];
} WifiNetwork;

typedef struct {
  WifiNetwork networks[MAX_WIFI_NETWORKS];
  int selected_network;
} AvailableWifiList;

#endif // STRUCTS_H
