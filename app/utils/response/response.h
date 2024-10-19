#ifndef RESPONSE_H
#define $NAME % _H

#include "../../app.h"

bool extract_response_text(char* text_box_store);
bool extract_status_line(App* app, char* status_line, size_t status_line_size);
bool extract_response_stream(char* text_box_store);
void prettify_json(App* app, char* output, size_t output_size);
void clear_new_lines(App* app);
bool is_json_response(const char* text);
bool is_json(App* app);
#endif // RESPONSE_H
