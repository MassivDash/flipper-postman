#ifndef APP_H
#define APP_H

#include "./structs.h"
#include "./uart/uart.h"
#include <gui/modules/dialog_ex.h>
#include <gui/modules/menu.h>
#include <gui/modules/number_input.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_input.h>
#include <gui/modules/variable_item_list.h>
#include <gui/scene_manager.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>

#define KEY_NAME_SIZE 22
#define TEXT_STORE_SIZE 40

typedef struct {
  SceneManager *scene_manager;
  ViewDispatcher *view_dispatcher;
  Menu *menu;
  DialogEx *dialog;
  FuriTimer *timer;
  Uart *uart;
  SetupDialogState dialog_state;
  uint8_t uart_ch;
} App;

#endif // APP_H