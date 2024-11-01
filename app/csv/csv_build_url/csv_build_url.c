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

void string_to_headers_c(const char* str, HttpBuildHeader headers[MAX_HEADERS]) {
    char* mutable_str = strdup(str);
    char* token;
    char* rest = mutable_str;
    int i = 0;

    while((token = strtok_r(rest, "@", &rest)) && i < MAX_HEADERS) {
        char* key = strtok(token, ":");
        char* value = strtok(NULL, ":");

        if(key) {
            strncpy(headers[i].key, key, TEXT_STORE_SIZE - 1);
            headers[i].key[TEXT_STORE_SIZE - 1] = '\0';

            if(value) {
                if(strcmp(value, EMPTY_VALUE) != 0) {
                    strncpy(headers[i].value, value, TEXT_STORE_SIZE - 1);
                    headers[i].value[TEXT_STORE_SIZE - 1] = '\0';
                } else {
                    headers[i].value[0] = '\0';
                }
            } else {
                headers[i].value[0] = '\0';
            }

            i++;
        }
    }

    // Clear any remaining headers
    for(; i < MAX_HEADERS; i++) {
        headers[i].key[0] = '\0';
        headers[i].value[0] = '\0';
    }

    free(mutable_str);
}

bool sync_csv_build_http_to_mem(App* app) {
    FURI_LOG_T(TAG, "Syncing Build HTTP CSV with flipper memory");
    char buffer[512]; // Stack-allocated buffer
    size_t bytes_read;

    if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open Build HTTP file");
        return false;
    }

    size_t index = 0;
    while(storage_file_read(app->file, buffer, sizeof(buffer) - 1) &&
          index < MAX_URLS_BUILD_HTTP) {
        bytes_read = strlen(buffer);
        buffer[bytes_read] = '\0';
        char* line = buffer;
        char* next_line;

        while((next_line = strchr(line, '\n')) != NULL && index < MAX_URLS_BUILD_HTTP) {
            *next_line = '\0';
            BuildHttpList* item = &app->build_http_list[index];
            char* token;
            int field = 0;

            // URL
            token = strsep(&line, ",");
            if(token) {
                strncpy(item->url, token, TEXT_STORE_SIZE - 1);
                item->url[TEXT_STORE_SIZE - 1] = '\0';
                field++;
            }

            // Mode
            token = strsep(&line, ",");
            if(token) {
                item->mode = atoi(token) != 0;
                field++;
            }

            // HTTP Method
            token = strsep(&line, ",");
            if(token) {
                item->http_method = (HttpBuildMethod)atoi(token);
                field++;
            }

            // Headers
            token = strsep(&line, ",");
            if(token) {
                if(strcmp(token, NO_HEADERS) != 0) {
                    string_to_headers_c(token, item->headers);
                } else {
                    memset(item->headers, 0, sizeof(HttpBuildHeader) * MAX_HEADERS);
                }
                field++;
            }

            // Payload
            token = strsep(&line, ",");
            if(token) {
                if(strcmp(token, NO_PAYLOAD) != 0) {
                    item->payload = furi_string_alloc_set(token);
                } else {
                    item->payload = furi_string_alloc();
                }
                field++;
            }

            // Show Response Headers
            token = strsep(&line, ",");
            if(token) {
                item->show_response_headers = atoi(token) != 0;
                field++;
            }

            if(field == 6) {
                index++;
            } else {
                FURI_LOG_W(TAG, "Invalid CSV line format");
            }

            line = next_line + 1;
        }

        // If there's remaining data, move it to the beginning of the buffer
        if(*line != '\0') {
            memmove(buffer, line, strlen(line) + 1);
            storage_file_seek(app->file, -(long)strlen(buffer), true);
        }
    }

    // Clear remaining entries
    for(; index < MAX_URLS_BUILD_HTTP; index++) {
        if(app->build_http_list[index].payload) {
            furi_string_free(app->build_http_list[index].payload);
        }
        memset(&app->build_http_list[index], 0, sizeof(BuildHttpList));
        app->build_http_list[index].payload = furi_string_alloc();
    }

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

bool write_build_http_to_csv(App* app, const BuildHttpList* item, bool has_headers) {
    FURI_LOG_T(TAG, "Writing Build HTTP to CSV");
    if(!storage_file_open(app->file, BUILD_HTTP_CSV_PATH, FSAM_WRITE, FSOM_OPEN_APPEND)) {
        FURI_LOG_E(TAG, "Failed to open Build HTTP file for writing");
        return false;
    }

    bool success = false;
    char buffer[256]; // Stack-allocated buffer
    int written = 0;
    int buffer_size = (int)sizeof(buffer);

    do {
        // Write URL, mode, and HTTP method
        written = snprintf(
            buffer,
            sizeof(buffer),
            "%s,%d,%d,",
            item->url,
            (int)item->mode,
            (int)item->http_method);
        if(written < 0 || written >= buffer_size) break;
        if(!storage_file_write(app->file, buffer, written)) break;

        // Write headers
        if(has_headers) {
            for(int i = 0; i < MAX_HEADERS && item->headers[i].key[0] != '\0'; i++) {
                written = snprintf(buffer, sizeof(buffer), "%s:", item->headers[i].key);
                if(written < 0 || written >= buffer_size) break;
                if(!storage_file_write(app->file, buffer, written)) break;

                if(item->headers[i].value[0] != '\0') {
                    written = snprintf(buffer, sizeof(buffer), "%s", item->headers[i].value);
                } else {
                    written = snprintf(buffer, sizeof(buffer), EMPTY_VALUE);
                }
                if(written < 0 || written >= buffer_size) break;
                if(!storage_file_write(app->file, buffer, written)) break;

                if(i < MAX_HEADERS - 1 && item->headers[i + 1].key[0] != '\0') {
                    if(!storage_file_write(app->file, "@", 1)) break;
                }
            }
        } else {
            if(!storage_file_write(app->file, NO_HEADERS, strlen(NO_HEADERS))) break;
        }
        if(!storage_file_write(app->file, ",", 1)) break;

        // Write payload
        if(item->payload && furi_string_size(item->payload) > 0) {
            if(!storage_file_write(
                   app->file, furi_string_get_cstr(item->payload), furi_string_size(item->payload)))
                break;
        } else {
            if(!storage_file_write(app->file, NO_PAYLOAD, strlen(NO_PAYLOAD))) break;
        }
        if(!storage_file_write(app->file, ",", 1)) break;

        // Write show_response_headers
        written = snprintf(buffer, sizeof(buffer), "%d\n", (int)item->show_response_headers);
        if(written < 0 || written >= buffer_size) break;
        if(!storage_file_write(app->file, buffer, written)) break;

        success = true;
    } while(0);

    if(!success) {
        FURI_LOG_E(TAG, "Failed to write Build HTTP to file");
    }

    storage_file_close(app->file);
    FURI_LOG_T(TAG, "Done with writing Build HTTP to CSV");
    return success;
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

        bool success = storage_file_write(
            app->file, furi_string_get_cstr(new_content), furi_string_size(new_content));

        if(!success) {
            FURI_LOG_E(TAG, "Failed to write updated content to Build HTTP file");
        }

        storage_file_close(app->file);

        if(!success) {
            furi_string_free(buffer);
            furi_string_free(new_content);
            return false;
        }
    }

    furi_string_free(buffer);
    furi_string_free(new_content);
    FURI_LOG_T(TAG, "Done with deleting Build HTTP from CSV");
    return url_found;
}
