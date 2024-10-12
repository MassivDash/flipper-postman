#include "../app.h"
#include <furi.h>
#include <stdlib.h>
#include <storage/storage.h>

#define TAG "wifi_app"

typedef struct {
  char ssid[32];
  char password[64];
  bool is_default;
} WifiCredential;

bool write_wifi_to_csv(App *app, const WifiCredential *wifi) {
  if (!storage_file_open(app->file, APP_DATA_PATH("wifi.csv"), FSAM_WRITE,
                         FSOM_OPEN_APPEND)) {
    FURI_LOG_E(TAG, "Failed to open file for writing");
    return false;
  }

  FuriString *buffer = furi_string_alloc();
  furi_string_printf(buffer, "%s,%s,%d\n", wifi->ssid, wifi->password,
                     wifi->is_default);

  if (!storage_file_write(app->file, furi_string_get_cstr(buffer),
                          furi_string_size(buffer))) {
    FURI_LOG_E(TAG, "Failed to write wifi to file");
    furi_string_free(buffer);
    storage_file_close(app->file);
    return false;
  }

  furi_string_free(buffer);
  storage_file_close(app->file);
  return true;
}

static bool read_line_from_file(App *app, FuriString *str_result) {
  furi_string_reset(str_result);
  uint8_t buffer[1];
  bool result = false;

  while (true) {
    size_t read_count = storage_file_read(app->file, buffer, sizeof(buffer));
    if (read_count == 0)
      break;
    if (buffer[0] == '\n') {
      result = true;
      break;
    } else {
      furi_string_push_back(str_result, buffer[0]);
    }
  }

  furi_string_push_back(str_result, '\0');
  return result;
}

bool read_wifi_from_csv(App *app) {
  FuriString *buffer = furi_string_alloc();
  if (!storage_file_open(app->file, APP_DATA_PATH("wifi.csv"), FSAM_READ,
                         FSOM_OPEN_EXISTING)) {
    FURI_LOG_E(TAG, "Failed to open file");
    furi_string_free(buffer);
    return false;
  }

  while (read_line_from_file(app, buffer)) {
    WifiCredential wifi = {0};
    const char *buffer_str = furi_string_get_cstr(buffer);
    sscanf(buffer_str, "%31[^,],%63[^,],%d", wifi.ssid, wifi.password,
           (int *)&wifi.is_default);
    // Process wifi credentials as needed
  }

  furi_string_free(buffer);
  storage_file_close(app->file);
  return true;
}

bool find_and_replace_wifi_in_csv(App *app,
                                  const WifiCredential *current_wifi) {
  FuriString *buffer_wifi = furi_string_alloc();
  FuriString *buffer_new_content = furi_string_alloc();
  bool wifi_found = false;

  if (!storage_file_open(app->file, APP_DATA_PATH("wifi.csv"), FSAM_READ,
                         FSOM_OPEN_EXISTING)) {
    FURI_LOG_E(TAG, "Failed to open file for reading");
    furi_string_free(buffer_wifi);
    furi_string_free(buffer_new_content);
    return false;
  }

  while (read_line_from_file(app, buffer_wifi)) {
    WifiCredential wifi = {0};
    const char *buffer_str = furi_string_get_cstr(buffer_wifi);
    sscanf(buffer_str, "%31[^,],%63[^,],%d", wifi.ssid, wifi.password,
           (int *)&wifi.is_default);

    if (strcmp(wifi.ssid, current_wifi->ssid) != 0) {
      furi_string_cat(buffer_new_content, buffer_str);
      furi_string_cat(buffer_new_content, "\n");
    } else {
      furi_string_printf(buffer_wifi, "%s,%s,%d\n", current_wifi->ssid,
                         current_wifi->password, current_wifi->is_default);
      furi_string_cat(buffer_new_content, furi_string_get_cstr(buffer_wifi));
      wifi_found = true;
    }
  }

  storage_file_close(app->file);

  if (wifi_found) {
    if (!storage_file_open(app->file, APP_DATA_PATH("wifi.csv"), FSAM_WRITE,
                           FSOM_CREATE_ALWAYS)) {
      FURI_LOG_E(TAG, "Failed to open file for writing");
      furi_string_free(buffer_wifi);
      furi_string_free(buffer_new_content);
      return false;
    }

    if (!storage_file_write(app->file, furi_string_get_cstr(buffer_new_content),
                            furi_string_size(buffer_new_content))) {
      FURI_LOG_E(TAG, "Failed to write new content to file");
      furi_string_free(buffer_wifi);
      furi_string_free(buffer_new_content);
      return false;
    }

    storage_file_close(app->file);
  }

  furi_string_free(buffer_wifi);
  furi_string_free(buffer_new_content);
  return wifi_found;
}

bool delete_wifi_from_csv(App *app, const char *ssid) {
  FuriString *buffer_wifi = furi_string_alloc();
  FuriString *buffer_new_content = furi_string_alloc();
  bool wifi_found = false;

  if (!storage_file_open(app->file, APP_DATA_PATH("wifi.csv"), FSAM_READ,
                         FSOM_OPEN_EXISTING)) {
    FURI_LOG_E(TAG, "Failed to open file for reading");
    furi_string_free(buffer_wifi);
    furi_string_free(buffer_new_content);
    return false;
  }

  while (read_line_from_file(app, buffer_wifi)) {
    WifiCredential wifi = {0};
    const char *buffer_str = furi_string_get_cstr(buffer_wifi);
    sscanf(buffer_str, "%31[^,],%63[^,],%d", wifi.ssid, wifi.password,
           (int *)&wifi.is_default);

    if (strcmp(wifi.ssid, ssid) != 0) {
      furi_string_cat(buffer_new_content, buffer_str);
      furi_string_cat(buffer_new_content, "\n");
    } else {
      wifi_found = true;
    }
  }

  storage_file_close(app->file);

  if (wifi_found) {
    if (!storage_file_open(app->file, APP_DATA_PATH("wifi.csv"), FSAM_WRITE,
                           FSOM_CREATE_ALWAYS)) {
      FURI_LOG_E(TAG, "Failed to open file for writing");
      furi_string_free(buffer_wifi);
      furi_string_free(buffer_new_content);
      return false;
    }

    if (!storage_file_write(app->file, furi_string_get_cstr(buffer_new_content),
                            furi_string_size(buffer_new_content))) {
      FURI_LOG_E(TAG, "Failed to write new content to file");
      furi_string_free(buffer_wifi);
      furi_string_free(buffer_new_content);
      return false;
    }

    storage_file_close(app->file);
  }

  furi_string_free(buffer_wifi);
  furi_string_free(buffer_new_content);
  return wifi_found;
}