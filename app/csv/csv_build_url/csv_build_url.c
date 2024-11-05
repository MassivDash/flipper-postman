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
#define NO_HEADERS          "NO_HEADERS"
#define NO_PAYLOAD          "NO_PAYLOAD"
#define EMPTY_VALUE         "EMPTY_VALUE"
#define MAX_BUFFER_SIZE     128
#define SMALL_BUFFER_SIZE   64

// Helper function to parse headers
void string_to_headers_c(const char* str, HttpBuildHeader headers[MAX_HEADERS]) {
    FuriString* token = furi_string_alloc();
    size_t str_len = strlen(str);
    size_t pos = 0;
    int i = 0;

    // Clear headers first
    memset(headers, 0, sizeof(HttpBuildHeader) * MAX_HEADERS);

    while(pos < str_len && i < MAX_HEADERS) {
        size_t delim_pos = strchr(str + pos, '@') - (str + pos);
        if(delim_pos > SMALL_BUFFER_SIZE) break;

        char key_buf[SMALL_BUFFER_SIZE] = {0};
        char value_buf[SMALL_BUFFER_SIZE] = {0};

        size_t kv_pos = strchr(str + pos, ':') - (str + pos);
        if(kv_pos > delim_pos || kv_pos > SMALL_BUFFER_SIZE) break;

        strncpy(key_buf, str + pos, kv_pos);
        strncpy(value_buf, str + pos + kv_pos + 1, delim_pos - kv_pos - 1);

        strncpy(headers[i].key, key_buf, TEXT_STORE_SIZE - 1);
        strncpy(headers[i].value, value_buf, TEXT_STORE_SIZE - 1);

        pos += delim_pos + 1;
        i++;
    }

    furi_string_free(token);
}

bool parse_csv_line(FuriString* buffer, BuildHttpList* item) {
    FuriString* token = furi_string_alloc();
    size_t pos = 0;
    bool success = false;

    // Parse URL
    pos = furi_string_search(buffer, ",", 0);
    if(pos == FURI_STRING_FAILURE) goto cleanup;
    furi_string_set(token, buffer);
    furi_string_left(token, pos);
    strncpy(item->url, furi_string_get_cstr(token), TEXT_STORE_SIZE - 1);
    furi_string_right(buffer, pos + 1);

    // Parse mode
    pos = furi_string_search(buffer, ",", 0);
    if(pos == FURI_STRING_FAILURE) goto cleanup;
    furi_string_set(token, buffer);
    furi_string_left(token, pos);
    item->mode = atoi(furi_string_get_cstr(token));
    furi_string_right(buffer, pos + 1);

    // Parse method
    pos = furi_string_search(buffer, ",", 0);
    if(pos == FURI_STRING_FAILURE) goto cleanup;
    furi_string_set(token, buffer);
    furi_string_left(token, pos);
    item->http_method = (HttpBuildMethod)atoi(furi_string_get_cstr(token));
    furi_string_right(buffer, pos + 1);

    success = true;

cleanup:
    furi_string_free(token);
    return success;
}

bool write_build_http_to_csv(App* app, const BuildHttpList* item, bool has_headers) {
    if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        return false;
    }

    // Prepare the entire line in memory
    FuriString* line = furi_string_alloc();
    furi_string_printf(line, "%s,%d,%d,", item->url, item->mode, item->http_method);

    // Append headers
    if(has_headers) {
        for(int i = 0; i < MAX_HEADERS && item->headers[i].key[0]; i++) {
            furi_string_cat_printf(
                line,
                "%s:%s%s",
                item->headers[i].key,
                item->headers[i].value[0] ? item->headers[i].value : EMPTY_VALUE,
                (i < MAX_HEADERS - 1 && item->headers[i + 1].key[0]) ? "@" : "");
        }
    } else {
        furi_string_cat_str(line, NO_HEADERS);
    }
    furi_string_cat_str(line, ",");

    // Append payload
    if(item->payload && furi_string_size(item->payload) > 0) {
        furi_string_cat(line, item->payload);
    } else {
        furi_string_cat_str(line, NO_PAYLOAD);
    }

    // Append final fields
    furi_string_cat_printf(line, ",%d\n", item->show_response_headers);

    FURI_LOG_D(TAG, "Writing line: %s", furi_string_get_cstr(line));
    // Write the entire line at once
    storage_file_write(app->file, furi_string_get_cstr(line), furi_string_size(line));

    furi_string_free(line);
    storage_file_close(app->file);
    return true;
}

bool sync_csv_build_http_to_mem(App* app) {
    FuriString* buffer = furi_string_alloc();

    if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        furi_string_free(buffer);
        return false;
    }

    size_t index = 0;
    while(index < MAX_URLS_BUILD_HTTP) {
        if(!read_line_from_file(app->file, buffer)) break;

        BuildHttpList* item = &app->build_http_list[index];
        memset(item, 0, sizeof(BuildHttpList));
        item->payload = furi_string_alloc();

        if(!parse_csv_line(buffer, item)) {
            furi_string_free(item->payload);
            break;
        }
        index++;
    }

    // Clear remaining entries
    for(; index < MAX_URLS_BUILD_HTTP; index++) {
        memset(&app->build_http_list[index], 0, sizeof(BuildHttpList));
        app->build_http_list[index].payload = furi_string_alloc();
    }

    furi_string_free(buffer);
    storage_file_close(app->file);
    return true;
}

bool init_csv_build_http(App* app) {
    if(!app->file) {
        FURI_LOG_E(TAG, "Failed to allocate storage file");
        return false;
    }

    // Initialize list with minimal memory
    for(int i = 0; i < MAX_URLS_BUILD_HTTP; i++) {
        memset(&app->build_http_list[i], 0, sizeof(BuildHttpList));
        app->build_http_list[i].payload = furi_string_alloc();
        if(!app->build_http_list[i].payload) {
            FURI_LOG_E(TAG, "Failed to allocate payload memory");
            // Cleanup previously allocated
            for(int j = 0; j < i; j++) {
                furi_string_free(app->build_http_list[j].payload);
            }
            return false;
        }
    }

    if(storage_file_exists(app->storage, BUILD_HTTP_CSV_PATH)) {
        if(!sync_csv_build_http_to_mem(app)) {
            FURI_LOG_E(TAG, "Failed to sync CSV data");
            return false;
        }
    } else {
        if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to create CSV file");
            return false;
        }
        storage_file_close(app->file);
    }

    return true;
}

bool delete_build_http_from_csv(App* app, const char* url) {
    FURI_LOG_T(TAG, "Deleting Build HTTP from CSV");

    if(!app || !url) return false;

    FuriString* buffer = furi_string_alloc();
    FuriString* new_content = furi_string_alloc();
    bool url_found = false;

    // Open file for reading
    if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open Build HTTP file for reading");
        furi_string_free(buffer);
        furi_string_free(new_content);
        return false;
    }

    // Read line by line and skip matching URL
    while(read_line_from_file(app->file, buffer)) {
        // Check if line starts with the URL we want to delete
        if(strncmp(furi_string_get_cstr(buffer), url, strlen(url)) != 0) {
            furi_string_cat(new_content, buffer);
            furi_string_cat_str(new_content, "\n");
        } else {
            url_found = true;
        }
    }

    storage_file_close(app->file);

    if(url_found) {
        // Rewrite file with filtered content
        if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            FURI_LOG_E(TAG, "Failed to open Build HTTP file for writing");
            furi_string_free(buffer);
            furi_string_free(new_content);
            return false;
        }

        if(furi_string_size(new_content) > 0) {
            if(!storage_file_write(
                   app->file, furi_string_get_cstr(new_content), furi_string_size(new_content))) {
                FURI_LOG_E(TAG, "Failed to write filtered content");
                storage_file_close(app->file);
                furi_string_free(buffer);
                furi_string_free(new_content);
                return false;
            }
        }

        storage_file_close(app->file);
    }

    furi_string_free(buffer);
    furi_string_free(new_content);

    FURI_LOG_T(TAG, "Done with deleting Build HTTP from CSV");
    return url_found;
}
