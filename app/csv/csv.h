#ifndef CSV_H
#define CSV_H

#include "../app.h"
#include "../structs.h"
#include <furi.h>
#include <storage/storage.h>

bool init_csv(App *app);
bool write_wifi_to_csv(App *app, const WifiCredential *wifi);
bool read_wifi_from_csv(App *app);
bool read_line_from_file(App *app, FuriString *str_result);
bool find_and_replace_wifi_in_csv(App *app, const WifiCredential *current_wifi);
bool delete_wifi_from_csv(App *app, const char *ssid);
bool sync_csv_to_mem(App *app);

#endif // CSV_H