#ifndef CSV_H
#define CSV_H

#include "../app.h"

typedef struct {
  char ssid[32];
  char password[64];
  bool is_default;
} WifiCredential;

bool write_wifi_to_csv(App *app, const WifiCredential *wifi);
bool read_wifi_from_csv(App *app);
bool find_and_replace_wifi_in_csv(App *app, const WifiCredential *current_wifi);
bool delete_wifi_from_csv(App *app, const char *ssid);

#endif // CSV_H