#include "../../app.h"
#include "../../structs.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/widget.h>

#define TAG "display_app"

typedef void (*DisplayCallback)(App* app, const char* header);

void get_the_header(App* app, char* header, size_t header_size) {
    DisplayMode mode = app->display_mode;
    GetState* get_state = app->get_state;
    switch(mode) {
    case DISPLAY_GET:
        snprintf(header, header_size, "Getting: %s", get_state->url);
        break;
    case DISPLAY_GET_STREAM:
        snprintf(header, header_size, "Getting Stream: %s", get_state->url);
        break;
    case DISPLAY_POST:
        snprintf(header, header_size, "Posting: %s", get_state->url);
        break;
    case DISPLAY_BUILD_HTTP:
        snprintf(header, header_size, "Building HTTP: %s", get_state->url);
        break;
    case DISPLAY_DOWNLOAD:
        snprintf(header, header_size, "Downloading: %s", get_state->url);
        break;
    case DISPLAY_LISTEN:
        snprintf(header, header_size, "Listening: %s", get_state->url);
        break;
    default:
        snprintf(header, header_size, "Unknown: %s", get_state->url);
        break;
    }
    // No return needed as header is an output parameter
}

void scene_on_enter_display(void* context) {
    App* app = context;
    furi_assert(app);
    char header[64];

    widget_reset(app->text_box);
    get_the_header(app, header, sizeof(header));

    widget_add_string_element(
        app->text_box,
        0, // x coordinate
        0, // y coordinate
        AlignLeft, // horizontal alignment
        AlignTop, // vertical alignment
        FontPrimary, // font
        header // text
    );
    // Switch to the Widget view
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Display);

    switch(app->display_mode) {
    case DISPLAY_GET_STREAM:
        FURI_LOG_I(TAG, "Getting stream");
        getCommand(app->uart, app->get_state->url);
        break;
    case DISPLAY_GET:
        FURI_LOG_I(TAG, "Getting data");
        getCommand(app->uart, app->get_state->url);
        break;
    case DISPLAY_POST:
        FURI_LOG_I(TAG, "Displaying POST view");
        break;
    case DISPLAY_BUILD_HTTP:
        FURI_LOG_I(TAG, "Displaying BUILD_HTTP view");
        break;
    case DISPLAY_DOWNLOAD:
        FURI_LOG_I(TAG, "Displaying DOWNLOAD view");
        break;
    case DISPLAY_LISTEN:
        FURI_LOG_I(TAG, "Displaying LISTEN view");
        break;
    default:
        FURI_LOG_E(TAG, "Unknown display mode");
        break;
    }

    // Add a header element

    // Add a scrollable text box element
    widget_add_text_scroll_element(
        app->text_box,
        0, // x coordinate
        16, // y coordinate (below the header)
        128, // width
        48, // height
        app->text_box_store // text
    );
}

void scene_on_exit_display(void* context) {
    App* app = context;
    furi_assert(app);
    FURI_LOG_D(TAG, "scene_on_exit_display");
    widget_reset(app->text_box);
    // Reset the text_box_store
    app->text_box_store[DISPLAY_STORE_SIZE] = '\0';
    app->display_mode = DISPLAY_NONE;
}
bool scene_on_event_display(void* context, SceneManagerEvent event) {
    App* app = context;
    furi_assert(app);
    furi_assert(event);

    bool consumed = false;
    // switch(event.type) {
    // case SceneManagerEventTypeBack:
    //     FURI_LOG_D(TAG, "Back event");
    //     if(app->display_mode == DISPLAY_GET_STREAM || app->display_mode == DISPLAY_GET) {
    //         app->display_mode =
    //             DISPLAY_NONE; // Assuming DISPLAY_NONE is a valid enum value representing no mode
    //         scene_manager_search_and_switch_to_previous_scene(app->scene_manager, Get);
    //         consumed = true;
    //     }
    //     break;
    // case SceneManagerEventTypeCustom:
    //     consumed = true;
    //     break;
    // case SceneManagerEventTypeTick:
    //     consumed = true;
    //     break;
    // default:
    //     consumed = false;
    //     break;
    // }

    return consumed;
}
