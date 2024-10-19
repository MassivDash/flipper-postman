#include "../../app.h"
#include "../../structs.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/widget.h>
#include "../../utils/response/response.h"

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
        bool success_stream = getCommand(app->uart, app->get_state->url);
        if(success_stream) {
            FURI_LOG_I(TAG, "Success");
            char status_line[64];
            if(extract_status_line(app, status_line, sizeof(status_line))) {
                FURI_LOG_I(TAG, "Status line: %s", status_line);
            } else {
                strncpy(status_line, "Unknown status", sizeof(status_line));
                status_line[sizeof(status_line) - 1] = '\0'; // Ensure null-termination
            }
            widget_reset(app->text_box);
            widget_add_string_element(
                app->text_box,
                0, // x coordinate
                0, // y coordinate
                AlignLeft, // horizontal alignment
                AlignTop, // vertical alignment
                FontPrimary, // font
                status_line // text
            );
            extract_response_stream(app);
        } else {
            FURI_LOG_I(TAG, "Failed");
        }
        break;
    case DISPLAY_GET:
        bool success = getCommand(app->uart, app->get_state->url);
        if(success) {
            FURI_LOG_I(TAG, "Success");
            char status_line[128];
            if(extract_status_line(app, status_line, sizeof(status_line))) {
                FURI_LOG_I(TAG, "Status line: %s", status_line);
            } else {
                strncpy(status_line, "Unknown status", sizeof(status_line));
                status_line[sizeof(status_line) - 1] = '\0'; // Ensure null-termination
            }

            clear_new_lines(app);

            extract_response_text(app);
            bool isJson = is_json_response(app);

            FURI_LOG_D("POSTMAN", "isJson: %d", isJson);

            if(isJson) {
                //Pretty print the JSON (to the best of flipper small screen ability)
                char pretty_json[DISPLAY_STORE_SIZE];
                prettify_json(app, pretty_json, sizeof(pretty_json));
                strncpy(app->text_box_store, pretty_json, DISPLAY_STORE_SIZE);
                app->text_box_store[DISPLAY_STORE_SIZE - 1] = '\0'; // Ensure null-termination

                // Add concat a (JSON VIEWER) to status line

                FuriString* furi_status_line = furi_string_alloc_set_str(status_line);
                furi_string_cat(furi_status_line, " (JSON)");
                strncpy(
                    status_line, furi_string_get_cstr(furi_status_line), sizeof(status_line) - 1);
                status_line[sizeof(status_line) - 1] = '\0'; // Ensure null-termination
                furi_string_free(furi_status_line);
            }

            widget_reset(app->text_box);
            widget_add_string_element(
                app->text_box,
                0, // x coordinate
                0, // y coordinate
                AlignLeft, // horizontal alignment
                AlignTop, // vertical alignment
                FontPrimary, // font
                status_line // text
            );

        } else {
            // Display error message, by coping the error message to the text_box_store
            strncpy(app->text_box_store, "Get Command failed", DISPLAY_STORE_SIZE);
            FURI_LOG_I(TAG, "Get Command failed");
        }
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
        14, // y coordinate (below the header)
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
    // app->text_box_store[0] = '\0';
    app->display_mode = DISPLAY_NONE;
}
bool scene_on_event_display(void* context, SceneManagerEvent event) {
    App* app = context;
    furi_assert(app);
    furi_assert(event);

    bool consumed = false;
    return consumed;
}
