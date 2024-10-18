#include "../../app.h"
#include "../../structs.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/text_box.h>

#define TAG "display_app"

void scene_on_enter_display(void* context) {
    App* app = context;
    furi_assert(app);

    TextBox* text_box = app->text_box;
    text_box_reset(text_box);
    text_box_set_font(text_box, TextBoxFontText);
    text_box_set_text(text_box, app->text_box_store);

    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Display);
}

bool scene_on_event_display(void* context, SceneManagerEvent event) {
    App* app = context;
    furi_assert(app);

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    }

    return consumed;
}

void scene_on_exit_display(void* context) {
    App* app = context;
    furi_assert(app);

    text_box_reset(app->text_box);
}
