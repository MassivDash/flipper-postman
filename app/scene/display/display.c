#include "../../app.h"
#include "../../structs.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/widget.h>

#define TAG "display_app"

void scene_on_enter_display(void* context) {
    App* app = context;
    furi_assert(app);
    char header[64];
    GetState* get_state = app->get_state;

    // Create header from get_state
    snprintf(header, sizeof(header), "Getting: %s", get_state->url);

    // Add a header element
    widget_add_string_element(
        app->text_box,
        0, // x coordinate
        0, // y coordinate
        AlignLeft, // horizontal alignment
        AlignTop, // vertical alignment
        FontPrimary, // font
        header // text
    );

    // Add a scrollable text box element
    widget_add_text_scroll_element(
        app->text_box,
        0, // x coordinate
        16, // y coordinate (below the header)
        128, // width
        48, // height
        app->text_box_store // text
    );

    // Switch to the Widget view
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
}
