#ifndef RESPONSE_H
#define RESPONSE_H

#include <furi.h>
#include <furi_hal.h>
#include "../../app.h" // Include the app header to access app structure

#ifdef __cplusplus
extern "C" {
#endif

size_t find_start_marker(const FuriString* furi_str, const char* start_marker);
size_t find_end_marker(const FuriString* furi_str, const char* end_marker, size_t start_pos);
bool extract_response_text(App* app);
void clear_new_lines(App* app);
bool extract_response_stream(App* app);
bool extract_status_line(App* app, FuriString* status_line);
bool is_json(App* app);
bool is_json_response(App* app);
bool prettify_json(App* app, FuriString* output);

#ifdef __cplusplus
}
#endif

#endif // RESPONSE_H
