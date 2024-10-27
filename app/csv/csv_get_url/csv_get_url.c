#include "csv_get_url.h"
#include "../csv_utils/csv_utils.h"
#include <furi.h>
#include <stdlib.h>
#include <storage/storage.h>
#include "../../app.h"

bool sync_csv_get_url_to_mem(App* app) {
    FURI_LOG_T(TAG, "Syncing CSV with flipper memory");
    FuriString* buffer = furi_string_alloc();

    if(!storage_file_open(app->file, APP_DATA_PATH("get_url.csv"), FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open file");
        furi_string_free(buffer);
        return false;
    }

    size_t url_index = 0;
    while(read_line_from_file(app->file, buffer) && url_index < MAX_URLS) {
        const char* buffer_str = furi_string_get_cstr(buffer);
        strncpy(app->url_list[url_index++].url, buffer_str, TEXT_STORE_SIZE - 1);
    }

    // Clear remaining entries in the array
    for(size_t i = url_index; i < MAX_URLS; i++) {
        memset(app->url_list[i].url, 0, TEXT_STORE_SIZE);
    }

    furi_string_free(buffer);
    storage_file_close(app->file);
    FURI_LOG_T(TAG, "Done with syncing CSV with flipper memory");
    return true;
}

bool write_url_to_csv(App* app, const char* url) {
    FURI_LOG_T(TAG, "Writing URL to CSV");
    if(!storage_file_open(app->file, APP_DATA_PATH("get_url.csv"), FSAM_WRITE, FSOM_OPEN_APPEND)) {
        FURI_LOG_E(TAG, "Failed to open file for writing");
        return false;
    }

    FuriString* buffer = furi_string_alloc();
    furi_string_printf(buffer, "%s\n", url);

    if(!storage_file_write(app->file, furi_string_get_cstr(buffer), furi_string_size(buffer))) {
        FURI_LOG_E(TAG, "Failed to write URL to file");
        furi_string_free(buffer);
        storage_file_close(app->file);
        return false;
    }

    furi_string_free(buffer);
    storage_file_close(app->file);
    FURI_LOG_T(TAG, "Done with writing URL to CSV");
    return true;
}

bool read_urls_from_csv(App* app) {
    FURI_LOG_T(TAG, "Reading URLs from CSV");

    FuriString* buffer = furi_string_alloc();
    if(!storage_file_open(app->file, APP_DATA_PATH("get_url.csv"), FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open file");
        furi_string_free(buffer);
        return false;
    }

    size_t url_index = 0;
    while(read_line_from_file(app->file, buffer) && url_index < MAX_URLS) {
        const char* buffer_str = furi_string_get_cstr(buffer);
        strncpy(app->url_list[url_index++].url, buffer_str, TEXT_STORE_SIZE - 1);
    }

    furi_string_free(buffer);
    storage_file_close(app->file);

    FURI_LOG_T(TAG, "Done with reading URLs from CSV");
    return true;
}

bool delete_url_from_csv(App* app, const char* url) {
    FURI_LOG_T(TAG, "Deleting URL from CSV");
    FuriString* buffer_url = furi_string_alloc();
    FuriString* buffer_new_content = furi_string_alloc();
    bool url_found = false;

    if(!storage_file_open(app->file, APP_DATA_PATH("get_url.csv"), FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open file for reading");
        furi_string_free(buffer_url);
        furi_string_free(buffer_new_content);
        return false;
    }

    while(read_line_from_file(app->file, buffer_url)) {
        const char* buffer_str = furi_string_get_cstr(buffer_url);

        if(strcmp(buffer_str, url) != 0) {
            furi_string_cat(buffer_new_content, buffer_str);
            furi_string_cat(buffer_new_content, "\n");
        } else {
            url_found = true;
        }
    }

    storage_file_close(app->file);

    if(url_found) {
        if(!storage_file_open(
               app->file, APP_DATA_PATH("get_url.csv"), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to open file for writing");
            furi_string_free(buffer_url);
            furi_string_free(buffer_new_content);
            storage_file_close(app->file);
            return false;
        }

        //if the new content is empty, just close the file and return

        if(furi_string_size(buffer_new_content) == 0) {
            storage_file_close(app->file);
            return true;
        }

        if(!storage_file_write(
               app->file,
               furi_string_get_cstr(buffer_new_content),
               furi_string_size(buffer_new_content))) {
            FURI_LOG_E(TAG, "Failed to write new content to file");
            furi_string_free(buffer_url);
            furi_string_free(buffer_new_content);
            storage_file_close(app->file);
            return false;
        }

        storage_file_close(app->file);
    }

    furi_string_free(buffer_url);
    furi_string_free(buffer_new_content);

    FURI_LOG_T(TAG, "Done with deleting URL from CSV");
    return url_found;
}

bool init_csv_get_url(App* app) {
    FURI_LOG_T(TAG, "Initializing CSV for get urls");

    // Allocate file
    // app->file = storage_file_alloc(app->storage);

    if(!app->file) {
        FURI_LOG_E(TAG, "Failed to allocate storage file");
        return false;
    }

    if(storage_file_exists(app->storage, APP_DATA_PATH("get_url.csv"))) {
        FURI_LOG_T(TAG, "URL CSV file exists");

        if(!sync_csv_get_url_to_mem(app)) {
            FURI_LOG_E(TAG, "Failed to sync CSV with flipper memory");
            storage_file_close(app->file);
            return false;
        }

    } else {
        FURI_LOG_T(TAG, "URL CSV file does not exist, creating a new one");

        if(!storage_file_open(
               app->file, APP_DATA_PATH("get_url.csv"), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to create CSV file");
            storage_file_close(app->file);
            return false;
        }
        storage_file_close(app->file);
    }

    FURI_LOG_T(TAG, "Done with initializing CSV for get urls");
    return true;
}
