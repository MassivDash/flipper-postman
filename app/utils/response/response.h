#ifndef RESPONSE_H
#define $NAME % _H

#include "../../app.h"

bool extract_response_text(App* app);
bool extract_status_line(App* app, char* status_line, size_t status_line_size);
void prettify_json(App* app, char* output, size_t output_size);
void clear_new_lines(App* app);
bool is_json_response(App* app);
bool is_json(App* app);
#endif // RESPONSE_H
