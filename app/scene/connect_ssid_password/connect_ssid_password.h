#ifndef CONNECT_SSID_PASSWORD_H
#define CONNECT_SSID_PASSWORD_H

#include <gui/elements.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>

#define HEADER_H 12U

typedef struct {
  char ssid[32];
  char password[32];
} WifiCredentials;

void draw_callback(Canvas *canvas, void *_model);
bool input_callback(InputEvent *event, void *context);
void scene_on_enter_connect_ssid_password(void *context);
bool scene_on_event_connect_ssid_password(void *context,
                                          SceneManagerEvent event);
void scene_on_exit_connect_ssid_password(void *context);

#endif // CONNECT_SSID_PASSWORD_H