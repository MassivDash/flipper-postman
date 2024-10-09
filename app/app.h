#ifndef APP_H
#define APP_H

#include "uart/uart.h"
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
  Submenu *submenu;
  View *view;
  File *file;
  DialogEx *dialog;
  FuriTimer *timer;
  TextInput *text_input;
  VariableItemList *variable_item_list;
  char text_store[TEXT_STORE_SIZE + 1];
  Postman postman; // Add Postman instance
} App;

#endif // APP_H