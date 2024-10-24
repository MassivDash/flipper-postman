#include "../../app.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <postmanflipx_icons.h>

typedef struct {
    size_t progress;
    FuriString* uart_message;
} DownloadProgressModel;

static void bytes_to_human_readable(size_t bytes, FuriString* output) {
    const char* suffixes[] = {"B", "KB", "MB", "GB", "TB"};
    size_t suffix_index = 0;
    double readable_size = (double)bytes;

    while(readable_size >= 1024 && suffix_index < (sizeof(suffixes) / sizeof(suffixes[0])) - 1) {
        readable_size /= 1024;
        suffix_index++;
    }

    furi_string_printf(output, "%.2f %s", readable_size, suffixes[suffix_index]);
}

static const uint8_t image_BLE_beacon_0_bits[] = {0x22, 0x49, 0x55, 0x49, 0x2a, 0x08, 0x08, 0x3e};

static void download_progress_draw_callback(Canvas* canvas, void* _model) {
    DownloadProgressModel* model = _model;
    furi_assert(model != NULL);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);

    canvas_set_bitmap_mode(canvas, true);
    canvas_draw_xbm(canvas, 8, 5, 7, 8, image_BLE_beacon_0_bits);

    canvas_draw_str(canvas, 21, 13, "downloading ...");

    FuriString* progress_text = furi_string_alloc();
    if(model->progress) {
        bytes_to_human_readable(model->progress, progress_text);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 10, 43, furi_string_get_cstr(progress_text));
    }

    if(model->uart_message != NULL) {
        FURI_LOG_T(TAG, "Uart message: %s", furi_string_get_cstr(model->uart_message));
        FURI_LOG_T(TAG, "Displaying UART message");
        canvas_set_font(canvas, FontSecondary);
        canvas_set_color(canvas, ColorBlack);
        elements_multiline_text_aligned(
            canvas, 10, 50, AlignCenter, AlignTop, furi_string_get_cstr(model->uart_message));
    }

    furi_string_free(progress_text);
}

static bool download_progress_input_callback(InputEvent* event, void* context) {
    App* app = context;
    UNUSED(app);
    UNUSED(event);
    if(event->key == InputKeyBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, Get);
        return true;
    }
    return false;
}

void update_download_progress(App* app, size_t progress) {
    with_view_model(
        app->view,
        DownloadProgressModel * model,
        {
            model->progress = progress;
            model->uart_message = app->text_box_store;
        },
        true);
}

void scene_on_enter_download_progress(void* context) {
    FURI_LOG_T(TAG, "Displaying DOWNLOAD view");
    App* app = context;
    view_allocate_model(app->view, ViewModelTypeLocking, sizeof(DownloadProgressModel));
    view_set_context(app->view, app);
    view_set_draw_callback(app->view, download_progress_draw_callback);
    view_set_input_callback(app->view, download_progress_input_callback);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Download);
    update_download_progress(app, 0);
    bool success_download = saveToFileCommand(app->uart, app->get_state->url);
    if(success_download) {
        FURI_LOG_I(TAG, "Download success");

    } else {
        FURI_LOG_E(TAG, "Download failed");
        // Copy error message to the text_box_store
    }
}

void scene_on_exit_download_progress(void* context) {
    App* app = context;
    app->save_to_file = false;
    furi_string_reset(app->text_box_store);
    view_free_model(app->view);
}

bool scene_on_event_download_progress(void* context, SceneManagerEvent event) {
    App* app = context;
    furi_assert(app);
    furi_assert(event);
    bool consumed = false;
    switch(event.type) {
    case(SceneManagerEventTypeBack):
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, Get);
        consumed = false;
        break;
    default:
        consumed = false;
        break;
    }

    return consumed;
}
