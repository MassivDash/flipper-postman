#ifndef CSV_BUILD_URL_H
#define CSV_BUILD_URL_H

#include "../../app.h"

bool init_csv_build_http(App* app);
bool sync_csv_build_http_to_mem(App* app);
bool delete_build_http_from_csv(App* app, const char* url);
bool write_build_http_to_csv(App* app, const BuildHttpList* item);

#endif // CSV_BUILD_URL_H