/** initialise app data, scene manager, and view dispatcher */

#include "../app.h"
#include "../scene/scene.h"
#include "../uart/uart.h" // Include the UART header file
#define TAG "postman_app"

App *init() {
  FURI_LOG_T(TAG, "init");
  App *app = malloc(sizeof(App));
  if (!app) {
    FURI_LOG_E(TAG, "Failed to allocate memory for App");
    return NULL;
  }
  scene_manager_init(app);
  view_dispatcher_init(app);
  return app;
}
