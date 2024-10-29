#include "./csv_build_url.h"
#include "../../app.h"
#include "../../structs.h"
#include "../csv_utils/csv_utils.h"
#include <furi.h>
#include <stdlib.h>
#include <storage/storage.h>

#define BUILD_HTTP_CSV_PATH APP_DATA_PATH("build_http.csv")
#define HEADER_DELIMITER    "@"
#define HEADER_KV_DELIMITER ":"

static void string_to_headers(FuriString* str, HttpBuildHeader headers[MAX_HEADERS]) {
    size_t start = 0;
    size_t end;
    size_t header_idx = 0;

    while((end = furi_string_search_char(str, '@', start)) != FURI_STRING_FAILURE &&
          header_idx < MAX_HEADERS) {
        size_t sep = furi_string_search_char(str, ':', start);
        if(sep < end && sep != FURI_STRING_FAILURE) {
            // Create FuriString for key and value
            FuriString* key = furi_string_alloc();
            FuriString* value = furi_string_alloc();

            // Set key and value
            furi_string_set_strn(key, furi_string_get_cstr(str) + start, sep - start);
            furi_string_set_strn(value, furi_string_get_cstr(str) + sep + 1, end - sep - 1);

            // Copy to HttpBuildHeader struct
            strncpy(headers[header_idx].key, furi_string_get_cstr(key), TEXT_STORE_SIZE - 1);
            strncpy(headers[header_idx].value, furi_string_get_cstr(value), TEXT_STORE_SIZE - 1);

            // Ensure null-termination
            headers[header_idx].key[TEXT_STORE_SIZE - 1] = '\0';
            headers[header_idx].value[TEXT_STORE_SIZE - 1] = '\0';

            header_idx++;

            // Free temporary FuriStrings
            furi_string_free(key);
            furi_string_free(value);
        }
        start = end + 1;
    }
}

static void headers_to_string(FuriString* result, const HttpBuildHeader headers[MAX_HEADERS]) {
    for(size_t i = 0; i < MAX_HEADERS; i++) {
        if(strlen(headers[i].key) > 0 && strlen(headers[i].value) > 0) {
            furi_string_cat_printf(result, "%s:%s@", headers[i].key, headers[i].value);
        }
    }
    // Remove the last delimiter
    if(furi_string_size(result) > 0) {
        furi_string_right(result, furi_string_size(result) - 1);
    }
}

bool sync_csv_build_http_to_mem(App* app) {
    FURI_LOG_T(TAG, "Syncing Build HTTP CSV with flipper memory");
    FuriString* buffer = furi_string_alloc();
    if(!buffer) {
        FURI_LOG_E(TAG, "Failed to allocate buffer");
        return false;
    }

    if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open Build HTTP file");
        furi_string_free(buffer);
        return false;
    }

    size_t index = 0;
    while(read_line_from_file(app->file, buffer) && index < MAX_URLS_BUILD_HTTP) {
        BuildHttpList* item = &app->build_http_list[index];
        const char* buffer_str = furi_string_get_cstr(buffer);
        char headers_str[1024] = {0}; // Adjust size as needed
        char payload_str[TEXT_STORE_SIZE] = {0};
        int mode, http_method, show_response_headers;

        sscanf(
            buffer_str,
            "%[^,],%d,%d,%[^,],%[^,],%d",
            item->url,
            &mode,
            &http_method,
            headers_str,
            payload_str,
            &show_response_headers);

        item->mode = (bool)mode;
        item->http_method = (HttpBuildMethod)http_method;
        item->show_response_headers = (bool)show_response_headers;

        FuriString* headers_furi_str = furi_string_alloc_set_str(headers_str);
        string_to_headers(headers_furi_str, item->headers);
        furi_string_free(headers_furi_str);

        furi_string_set(item->payload, payload_str);

        index++;
    }

    // Clear remaining entries
    for(; index < MAX_URLS_BUILD_HTTP; index++) {
        memset(&app->build_http_list[index], 0, sizeof(BuildHttpList));
        app->build_http_list[index].payload = furi_string_alloc();
    }

    furi_string_free(buffer);
    storage_file_close(app->file);
    FURI_LOG_T(TAG, "Done with syncing Build HTTP CSV with flipper memory");
    return true;
}

bool init_csv_build_http(App* app) {
    FURI_LOG_T(TAG, "Initializing Build HTTP CSV");

    if(!app->file) {
        FURI_LOG_E(TAG, "Failed to allocate storage file");
        return false;
    }

    // Initialize the build_http_list array
    for(int i = 0; i < MAX_URLS_BUILD_HTTP; i++) {
        memset(&app->build_http_list[i], 0, sizeof(BuildHttpList));
        app->build_http_list[i].payload = furi_string_alloc();
        if(!app->build_http_list[i].payload) {
            FURI_LOG_E(TAG, "Failed to allocate memory for payload");
            // Clean up previously allocated memory
            for(int j = 0; j < i; j++) {
                furi_string_free(app->build_http_list[j].payload);
            }
            return false;
        }
    }

    if(storage_file_exists(app->storage, BUILD_HTTP_CSV_PATH)) {
        FURI_LOG_I(TAG, "Build HTTP CSV file exists, reading data from file");
        if(!sync_csv_build_http_to_mem(app)) {
            FURI_LOG_E(TAG, "Failed to read Build HTTP data from CSV file");
            return false;
        }
    } else {
        FURI_LOG_I(TAG, "Build HTTP CSV file does not exist, creating a new one.");
        if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to create Build HTTP CSV file");
            return false;
        }
        storage_file_close(app->file);
    }

    FURI_LOG_T(TAG, "Done with initializing Build HTTP CSV");
    return true;
}

bool write_build_http_to_csv(App* app, const BuildHttpList* item) {
    FURI_LOG_T(TAG, "Writing Build HTTP to CSV");
    if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        FURI_LOG_E(TAG, "Failed to open Build HTTP file for writing");
        return false;
    }

    FuriString* buffer = furi_string_alloc();
    FuriString* headers_str = furi_string_alloc();

    headers_to_string(headers_str, item->headers);

    furi_string_printf(
        buffer,
        "%s,%d,%d,%s,%s,%d\n",
        item->url,
        (int)item->mode,
        (int)item->http_method,
        furi_string_get_cstr(headers_str),
        furi_string_get_cstr(item->payload),
        (int)item->show_response_headers);

    if(!storage_file_write(app->file, furi_string_get_cstr(buffer), furi_string_size(buffer))) {
        FURI_LOG_E(TAG, "Failed to write Build HTTP to file");
        furi_string_free(buffer);
        furi_string_free(headers_str);
        storage_file_close(app->file);
        return false;
    }

    furi_string_free(buffer);
    furi_string_free(headers_str);
    storage_file_close(app->file);
    FURI_LOG_T(TAG, "Done with writing Build HTTP to CSV");
    return true;
}

bool delete_build_http_from_csv(App* app, const char* url) {
    FURI_LOG_T(TAG, "Deleting Build HTTP from CSV");
    FuriString* buffer = furi_string_alloc();
    FuriString* new_content = furi_string_alloc();
    bool url_found = false;

    if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open Build HTTP file for reading");
        furi_string_free(buffer);
        furi_string_free(new_content);
        return false;
    }

    while(read_line_from_file(app->file, buffer)) {
        if(strncmp(furi_string_get_cstr(buffer), url, strlen(url)) != 0) {
            furi_string_cat(new_content, buffer);
            furi_string_cat(new_content, "\n");
        } else {
            url_found = true;
        }
    }

    storage_file_close(app->file);

    if(url_found) {
        if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to open Build HTTP file for writing");
            furi_string_free(buffer);
            furi_string_free(new_content);
            return false;
        }

        if(!storage_file_write(
               app->file, furi_string_get_cstr(new_content), furi_string_size(new_content))) {
            FURI_LOG_E(TAG, "Failed to write updated content to Build HTTP file");
            furi_string_free(buffer);
            furi_string_free(new_content);
            storage_file_close(app->file);
            return false;
        }

        storage_file_close(app->file);
    }

    furi_string_free(buffer);
    furi_string_free(new_content);
    FURI_LOG_T(TAG, "Done with deleting Build HTTP from CSV");
    return url_found;
}
