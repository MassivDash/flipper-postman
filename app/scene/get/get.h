#ifndef GET_H
#define GET_H

#include "../../app.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/variable_item_list.h>

// Enum for variable item list items
typedef enum {
    GetItemHeader,
    GetItemSetUrl,
    GetItemToggleViewSave,
    GetItemAction,
    GetItemCount
} GetItem;

// Function prototypes
void scene_on_enter_get(void* context);
bool scene_on_event_get(void* context, SceneManagerEvent event);
void scene_on_exit_get(void* context);

#endif // GET_H