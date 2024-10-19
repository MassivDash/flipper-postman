#ifndef CSV_H
#define CSV_H

#include "../app.h"
#include "../structs.h"
#include <furi.h>
#include <storage/storage.h>

bool init_csv(File* file, WifiCredential* csv_networks);
bool write_wifi_to_csv(File* file, const WifiCredential* wifi);
bool read_wifi_from_csv(File* file, WifiCredential* csv_networks);
bool read_line_from_file(File* file, FuriString* str_result);
bool find_and_replace_wifi_in_csv(File* file, const WifiCredential* current_wifi);
bool delete_wifi_from_csv(File* file, const char* ssid);
bool sync_csv_to_mem(File* file, WifiCredential* csv_networks);

#endif // CSV_H
