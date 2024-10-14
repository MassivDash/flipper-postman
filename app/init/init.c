#include "../app.h"
#include "../csv/csv.h"
#include "../scene/scene.h"
#include "../uart/uart.h"

#define TAG "postman_app"

App *init() {
  FURI_LOG_T(TAG, "init");
  App *app = malloc(sizeof(App));
  if (!app) {
    FURI_LOG_E(TAG, "Failed to allocate memory for App");
    return NULL;
  }
  app->uart = uart_terminal_uart_init(app);

  if (!init_csv(app)) {
    FURI_LOG_E(TAG, "Failed to initialize CSV");
    free(app);
    return NULL;
  }

  if (!app->uart) {
    FURI_LOG_E(TAG, "Failed to initialize UART");
    free(app);
    return NULL;
  }

  scene_manager_init(app);
  view_dispatcher_init(app);

  return app;
}