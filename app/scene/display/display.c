#include "../../app.h"
#include "../../structs.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/modules/widget.h>
#include "../../utils/response/response.h"

typedef void (*DisplayCallback)(App* app, const char* header);

void get_the_header(App* app, FuriString* header) {
    DisplayMode mode = app->display_mode;
    GetState* get_state = app->get_state;
    furi_string_reset(header);
    switch(mode) {
    case DISPLAY_GET:
        furi_string_printf(header, "Getting: %s", get_state->url);
        break;
    case DISPLAY_GET_STREAM:
        furi_string_printf(header, "Getting Stream: %s", get_state->url);
        break;
    case DISPLAY_POST:
        furi_string_printf(header, "Posting: %s", get_state->url);
        break;
    case DISPLAY_BUILD_HTTP:
        furi_string_printf(header, "Building HTTP: %s", get_state->url);
        break;
    case DISPLAY_DOWNLOAD:
        furi_string_printf(header, "Downloading: %s", get_state->url);
        break;
    case DISPLAY_LISTEN:
        furi_string_printf(header, "Listening: %s", get_state->url);
        break;
    default:
        furi_string_printf(header, "Unknown: %s", get_state->url);
        break;
    }
}

static void append_to_status_line(FuriString* status_line, const char* append_str) {
    furi_string_cat(status_line, append_str);
}

void scene_on_enter_display(void* context) {
    App* app = context;
    furi_assert(app);
    FuriString* header = furi_string_alloc();

    widget_reset(app->text_box);
    get_the_header(app, header);

    widget_add_string_element(
        app->text_box,
        0, // x coordinate
        0, // y coordinate
        AlignLeft, // horizontal alignment
        AlignTop, // vertical alignment
        FontPrimary, // font
        furi_string_get_cstr(header) // text
    );
    // Switch to the Widget view
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Display);

    switch(app->display_mode) {
    case DISPLAY_GET_STREAM:
        FURI_LOG_T(TAG, "GET_STREAM DISPLAY ACTION");
        bool success_stream = getCommand(app->uart, app->get_state->url);
        if(success_stream) {
            FURI_LOG_T(TAG, "GET_STREAM DISPLAY ACTION SUCCESS");
            FuriString* status_line = furi_string_alloc();
            if(extract_status_line(app, status_line)) {
                FURI_LOG_T(
                    TAG,
                    "GET_STREAM DISPLAY RESPONSE (status) %s",
                    furi_string_get_cstr(status_line));
            } else {
                FURI_LOG_E(TAG, "GET_STREAM DISPLAY ERROR, DID NOT CATCH STATUS");
                furi_string_set_str(status_line, "STATUS: Unknown or -1");
            }

            // Add (Direct) to status line
            append_to_status_line(status_line, " (Direct)");

            widget_reset(app->text_box);
            widget_add_string_element(
                app->text_box,
                0, // x coordinate
                0, // y coordinate
                AlignLeft, // horizontal alignment
                AlignTop, // vertical alignment
                FontPrimary, // font
                furi_string_get_cstr(status_line) // text
            );
            furi_string_free(status_line);
        } else {
            FURI_LOG_E(TAG, "GET STREAM DISPLAY ACTION FAILED");
        }
        break;
    case DISPLAY_GET:
        bool success = getCommand(app->uart, app->get_state->url);
        if(success) {
            FURI_LOG_I(TAG, "Success");
            FuriString* status_line = furi_string_alloc();
            if(extract_status_line(app, status_line)) {
                FURI_LOG_I(TAG, "Status line: %s", furi_string_get_cstr(status_line));
            } else {
                furi_string_set_str(status_line, "Unknown status");
            }

            clear_new_lines(app);

            if(extract_response_text(app, "RESPONSE: ", " RESPONSE_END")) {
                FURI_LOG_I(TAG, "Response text extracted");
            } else {
                FURI_LOG_E(TAG, "Failed to extract response text");
                // In the instance of content length being -1, or huge response, extract the stream
                if(extract_response_text(app, "STREAM: ", " STREAM_END")) {
                    FURI_LOG_I(TAG, "Stream extracted");
                } else {
                    FURI_LOG_E(TAG, "Failed to extract stream");
                }
            }

            bool isJson = is_json_response(app);

            FURI_LOG_D("POSTMAN", "isJson: %d", isJson);

            // Pretty print the JSON (to the best of flipper small screen ability)
            if(isJson) {
                FuriString* pretty_json = furi_string_alloc();
                if(prettify_json(app, pretty_json)) {
                    furi_string_set(app->text_box_store, pretty_json);

                    // Add concat a (JSON VIEWER) to status line
                    // Append "(JSON)" to the status line
                    append_to_status_line(status_line, " (JSON)");
                } else {
                    // Fallback if prettification fails
                    FURI_LOG_W(TAG, "JSON prettification failed");
                }
                furi_string_free(pretty_json);
            }

            widget_reset(app->text_box);
            widget_add_string_element(
                app->text_box,
                0, // x coordinate
                0, // y coordinate
                AlignLeft, // horizontal alignment
                AlignTop, // vertical alignment
                FontPrimary, // font
                furi_string_get_cstr(status_line) // text
            );
            furi_string_free(status_line);
        } else {
            // Display error message, by coping the error message to the text_box_store
            furi_string_set_str(app->text_box_store, "Get Command failed");
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
        bool success_download = saveToFileCommand(app->uart, app->get_state->url);
        if(success_download) {
            FURI_LOG_I(TAG, "Download success");
        } else {
            FURI_LOG_E(TAG, "Download failed");
        }
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
        furi_string_get_cstr(app->text_box_store) // text
    );

    furi_string_free(header);
}

void scene_on_exit_display(void* context) {
    App* app = context;
    furi_assert(app);
    FURI_LOG_D(TAG, "scene_on_exit_display");
    widget_reset(app->text_box);
    // Reset the text_box_store
    app->save_to_file = false;
    app->filename[0] = '\0';
    furi_string_reset(app->text_box_store);
    app->display_mode = DISPLAY_NONE;
}

bool scene_on_event_display(void* context, SceneManagerEvent event) {
    FURI_LOG_T(TAG, "scene_on_event_display");
    App* app = context;
    furi_assert(app);
    furi_assert(event);
    bool consumed = false;
    switch(event.type) {
    case(SceneManagerEventTypeBack):
        if(app->display_mode == DISPLAY_DOWNLOAD) {
            scene_manager_search_and_switch_to_another_scene(app->scene_manager, Get);
            consumed = true;
        }
        break;
    default:
        consumed = false;
        break;
    }

    return consumed;
}
