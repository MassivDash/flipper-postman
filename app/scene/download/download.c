#include "../../app.h"
#include <furi.h>
#include <furi_hal.h>
#include <gui/elements.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>

typedef struct {
    int progress;
    FuriString* uart_message;
} DownloadProgressModel;

static void download_progress_draw_callback(Canvas* canvas, void* _model) {
    DownloadProgressModel* model = _model;
    furi_assert(model != NULL);

    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontPrimary);
    elements_multiline_text_aligned(
        canvas, canvas_width(canvas) / 2, 0, AlignCenter, AlignTop, "Downloading...");

    char progress_text[16];
    snprintf(progress_text, sizeof(progress_text), "Progress: %d/100", model->progress);
    canvas_draw_str_aligned(
        canvas, canvas_width(canvas) / 2, 20, AlignCenter, AlignTop, progress_text);

    if(model->uart_message != NULL) {
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontPrimary);
        elements_multiline_text_aligned(
            canvas,
            canvas_width(canvas) / 2,
            40,
            AlignCenter,
            AlignTop,
            furi_string_get_cstr(model->uart_message));
    }
}

static bool download_progress_input_callback(InputEvent* event, void* context) {
    App* app = context;
    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, AppView_Display);
        return true;
    }
    return false;
}

void scene_on_enter_download_progress(void* context) {
    FURI_LOG_T(TAG, "Displaying DOWNLOAD view");
    App* app = context;
    view_allocate_model(app->view, ViewModelTypeLocking, sizeof(DownloadProgressModel));
    view_set_context(app->view, app);
    view_set_draw_callback(app->view, download_progress_draw_callback);
    view_set_input_callback(app->view, download_progress_input_callback);
    view_dispatcher_switch_to_view(app->view_dispatcher, AppView_Download);
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
    view_free_model(app->view);
}

bool scene_on_event_download_progress(void* context, SceneManagerEvent event) {
    App* app = context;
    UNUSED(app);
    if(event.type == SceneManagerEventTypeCustom) {
        // Handle custom events if needed
    }
    return false;
}

void update_download_progress(App* app, int progress) {
    with_view_model(
        app->view,
        DownloadProgressModel * model,
        {
            model->progress = progress;
            model->uart_message = app->text_box_store;
        },
        true);
}
