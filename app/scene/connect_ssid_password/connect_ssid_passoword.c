#include "../../app.h"
#include "../../structs.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>

#define TAG "connect_ssid_password"
#define HEADER_H 12U

typedef struct {
  char ssid[32];
  char password[32];
} WifiCredentials;

static void draw_callback(Canvas *canvas, void *_model) {
  WifiCredentials *model = _model;
  furi_assert(model != NULL, "Model is NULL");

  canvas_clear(canvas);
  canvas_set_color(canvas, ColorBlack);
  canvas_set_font(canvas, FontPrimary);
  elements_multiline_text_aligned(canvas, canvas_width(canvas) / 2, 0,
                                  AlignCenter, AlignTop, "Connect to WiFi");

  canvas_draw_str(canvas, 4, HEADER_H + 10, "SSID:");
  canvas_draw_str(canvas, 4, HEADER_H + 30, model->ssid);

  canvas_draw_str(canvas, 4, HEADER_H + 50, "Password:");
  canvas_draw_str(canvas, 4, HEADER_H + 70, model->password);
}

static bool input_callback(InputEvent *event, void *context) {
  App *app = context;

  if (event->type == InputTypeShort) {
    if (event->key == InputKeyBack) {
      scene_manager_search_and_switch_to_previous_scene(app->scene_manager,
                                                        Connect_Details);
      return false; // Pass the back event through
    }
  }

  return false;
}

void scene_on_enter_connect_ssid_password(void *context) {
  App *app = context;
  view_allocate_model(app->view, ViewModelTypeLocking, sizeof(WifiCredentials));
  view_set_context(app->view, app);
  view_set_draw_callback(app->view, draw_callback);
  view_set_input_callback(app->view, input_callback);
  view_dispatcher_switch_to_view(app->view_dispatcher,
                                 AppView_Connect_Ssid_Password);
}

bool scene_on_event_connect_ssid_password(void *context,
                                          SceneManagerEvent event) {
  App *app = context;
  bool consumed = false;
  if (event.type == SceneManagerEventTypeCustom) {
    switch (event.event) {
    case DialogExResultCenter:
      consumed = true;
      // Handle center button press if needed
      break;
    case DialogExResultLeft:
      scene_manager_handle_back_event(app->scene_manager);
      consumed = true;
      break;
    default:
      consumed = false;
      break;
    }
  }
  return consumed;
}

void scene_on_exit_connect_ssid_password(void *context) {
  App *app = context;
  view_free_model(app->view);
}