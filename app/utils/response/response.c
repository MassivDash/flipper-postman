#include <furi.h>
#include <furi_hal.h>
#include <string.h>
#include <stdio.h>
#include "../../app.h" // Include the app header to access app structure

#define TAG "RESPONSE"
// bool extract_response_text(App* app) {
//     const char* start_marker = "RESPONSE:";
//     const char* end_marker = "RESPONSE_END";
//     const size_t start_marker_len = strlen(start_marker);
//     FuriString* furi_str = furi_string_alloc_set_str(app->text_box_store);
//     size_t text_length = furi_string_size(furi_str);
//     const char* text = furi_string_get_cstr(furi_str);

//     // Find start marker
//     size_t start_pos = furi_string_search_str(furi_str, start_marker, 0);
//     if(start_pos == FURI_STRING_FAILURE) {
//         furi_string_free(furi_str);
//         return false;
//     }

//     // Move past the start marker
//     start_pos += start_marker_len;

//     // Search for end marker starting from the position after start marker
//     size_t end_pos = furi_string_search_str(furi_str, end_marker, start_pos);

//     // Calculate extraction length
//     size_t extract_length;
//     if(end_pos == FURI_STRING_FAILURE) {
//         // If no end marker found, take everything until the end
//         extract_length = text_length - start_pos;
//     } else {
//         extract_length = end_pos - start_pos;
//     }

//     // Safety check for buffer overflow
//     if(extract_length >= DISPLAY_STORE_SIZE) {
//         extract_length = DISPLAY_STORE_SIZE - 1;
//     }

//     // Extract the content
//     memcpy(app->text_box_store, text + start_pos, extract_length);
//     app->text_box_store[extract_length] = '\0';

//     furi_string_free(furi_str);
//     return true;
// }

size_t find_start_marker(const FuriString* furi_str, const char* start_marker) {
    size_t start_pos = furi_string_search_str(furi_str, start_marker, 0);
    if(start_pos != FURI_STRING_FAILURE) {
        FURI_LOG_T(TAG, "Start marker found at position: %zu", start_pos);
    } else {
        FURI_LOG_T(TAG, "Start marker not found");
    }
    return start_pos;
}

size_t find_end_marker(const FuriString* furi_str, const char* end_marker, size_t start_pos) {
    size_t end_pos = furi_string_search_str(furi_str, end_marker, start_pos);
    if(end_pos != FURI_STRING_FAILURE) {
        FURI_LOG_T(TAG, "End marker found at position: %zu", end_pos);
    } else {
        FURI_LOG_T(TAG, "End marker not found");
    }
    return end_pos;
}

bool extract_response_text(App* app) {
    const char* start_marker = "RESPONSE: ";
    const char* end_marker = " RESPONSE_END";

    FuriString* furi_str = furi_string_alloc_set_str(app->text_box_store);
    size_t start_pos = find_start_marker(furi_str, start_marker);
    size_t end_pos = find_end_marker(furi_str, end_marker, start_pos);

    // If the start marker is found
    if(start_pos != FURI_STRING_FAILURE) {
        start_pos += strlen(start_marker); // Move past the start marker

        // If the end marker is found and is after the start marker
        if(end_pos != FURI_STRING_FAILURE && end_pos > start_pos) {
            size_t length = end_pos - start_pos; // Calculate the length of the text to extract

            // Extract the text between the markers
            FuriString* extracted_str = furi_string_alloc();
            furi_string_set_n(extracted_str, furi_str, start_pos, length);

            // Trim leading and trailing spaces
            furi_string_trim(extracted_str, " ");

            // Copy the extracted text back to text_box_store
            strncpy(
                app->text_box_store, furi_string_get_cstr(extracted_str), DISPLAY_STORE_SIZE - 1);
            app->text_box_store[DISPLAY_STORE_SIZE - 1] = '\0'; // Ensure null-termination

            // Free the allocated FuriStrings
            furi_string_free(furi_str);
            furi_string_free(extracted_str);

            return true;
        } else {
            // If no end marker is found, extract till the end of the string
            const char* remaining_text = furi_string_get_cstr(furi_str) + start_pos;
            FuriString* remaining_str = furi_string_alloc_set_str(remaining_text);

            // Trim leading and trailing spaces
            furi_string_trim(remaining_str, " ");

            strncpy(
                app->text_box_store, furi_string_get_cstr(remaining_str), DISPLAY_STORE_SIZE - 1);
            app->text_box_store[DISPLAY_STORE_SIZE - 1] = '\0'; // Ensure null-termination

            // Free the allocated FuriStrings
            furi_string_free(furi_str);
            furi_string_free(remaining_str);

            return true;
        }
    }

    // Free the allocated FuriString
    furi_string_free(furi_str);

    return false; // No valid response text found
}

void clear_new_lines(App* app) {
    FuriString* furi_str = furi_string_alloc_set_str(app->text_box_store);
    FuriString* cleaned_str = furi_string_alloc();

    for(size_t i = 0; i < furi_string_size(furi_str); i++) {
        char ch = furi_string_get_char(furi_str, i);
        if(ch == '\n' || ch == '\r' || ch == '\t') {
            furi_string_push_back(cleaned_str, ' '); // Replace new line with space
        } else {
            furi_string_push_back(cleaned_str, ch);
        }
    }

    // Copy the cleaned text back to text_box_store
    strncpy(
        app->text_box_store, furi_string_get_cstr(cleaned_str), sizeof(app->text_box_store) - 1);
    app->text_box_store[sizeof(app->text_box_store) - 1] = '\0'; // Ensure null-termination

    // Free the allocated FuriStrings
    furi_string_free(furi_str);
    furi_string_free(cleaned_str);
}

bool extract_response_stream(App* app) {
    const char* start_marker = "STREAM:";
    const char* end_marker = "STREAM_END";

    FuriString* furi_str = furi_string_alloc_set_str(app->text_box_store);
    size_t start_pos = furi_string_search_str(furi_str, start_marker, 0);
    size_t end_pos = furi_string_search_str(furi_str, end_marker, 0);

    // If both markers are found and in the correct order
    if(start_pos != FURI_STRING_FAILURE && end_pos != FURI_STRING_FAILURE && start_pos < end_pos) {
        start_pos += strlen(start_marker); // Move past the start marker
        size_t length = end_pos - start_pos; // Calculate the length of the text to extract

        // Extract the text between the markers
        FuriString* extracted_str = furi_string_alloc();
        furi_string_set_n(extracted_str, furi_str, start_pos, length);

        // Copy the extracted text back to text_box_store
        strncpy(app->text_box_store, furi_string_get_cstr(extracted_str), DISPLAY_STORE_SIZE - 1);
        app->text_box_store[DISPLAY_STORE_SIZE - 1] = '\0'; // Ensure null-termination

        // Free the allocated FuriStrings
        furi_string_free(furi_str);
        furi_string_free(extracted_str);

        return true;
    }

    // Free the allocated FuriString
    furi_string_free(furi_str);

    return false; // No valid stream command found
}

bool extract_status_line(App* app, char* status_line, size_t status_line_size) {
    const char* status_marker = "STATUS: ";
    FuriString* furi_str = furi_string_alloc_set_str(app->text_box_store);
    size_t start_pos = 0;

    while((start_pos = furi_string_search_str(furi_str, status_marker, start_pos)) !=
          FURI_STRING_FAILURE) {
        start_pos += strlen(status_marker);

        // Ensure there are at least three characters after the status marker
        if(furi_string_size(furi_str) >= start_pos + 3) {
            // Copy the entire status line including the marker and the next three characters
            snprintf(
                status_line,
                status_line_size,
                "%s%.3s",
                status_marker,
                furi_string_get_cstr(furi_str) + start_pos);
            furi_string_free(furi_str);
            return true;
        }

        // Move to the next character to continue searching
        start_pos++;
    }

    furi_string_free(furi_str);
    return false; // No valid status line found
}

bool is_json(App* app) {
    // Trim leading and trailing whitespace and null characters
    FuriString* furi_str = furi_string_alloc_set_str(app->text_box_store);
    furi_string_trim(furi_str, " \t\n\r\0");

    const char* trimmed_text = furi_string_get_cstr(furi_str);
    size_t len = strlen(trimmed_text);

    bool result = false;
    if(len > 0 && ((trimmed_text[0] == '{' && trimmed_text[len - 1] == '}') ||
                   (trimmed_text[0] == '[' && trimmed_text[len - 1] == ']'))) {
        result = true;
    }

    furi_string_free(furi_str);
    return result;
}

bool is_json_response(App* app) {
    // Check if the text contains JSON-like structures
    const char* packet_marker = "1e38";
    const char* start = app->text_box_store;

    while((start = strstr(start, packet_marker)) != NULL) {
        start += strlen(packet_marker);
        if(is_json(app)) {
            return true;
        }
    }

    // Check the whole string if no packet markers are found
    return is_json(app);
}

void prettify_json(App* app, char* output, size_t output_size) {
    size_t indent = 0;
    size_t output_index = 0;
    bool in_string = false;

    for(size_t i = 0; i < strlen(app->text_box_store) && output_index < output_size - 1; i++) {
        char ch = app->text_box_store[i];

        switch(ch) {
        case '{':
        case '[':
            if(!in_string) {
                output[output_index++] = ch;
                output[output_index++] = '\n';
                indent++;
                for(size_t j = 0; j < indent; j++) {
                    output[output_index++] = ' ';
                }
            } else {
                output[output_index++] = ch;
            }
            break;
        case '}':
        case ']':
            if(!in_string) {
                output[output_index++] = '\n';
                indent--;
                for(size_t j = 0; j < indent; j++) {
                    output[output_index++] = ' ';
                }
                output[output_index++] = ch;
            } else {
                output[output_index++] = ch;
            }
            break;
        case ',':
            if(!in_string) {
                output[output_index++] = ch;
                output[output_index++] = '\n';
                for(size_t j = 0; j < indent; j++) {
                    output[output_index++] = ' ';
                }
            } else {
                output[output_index++] = ch;
            }
            break;
        case ':':
            if(!in_string) {
                output[output_index++] = ch;
                output[output_index++] = ' ';
            } else {
                output[output_index++] = ch;
            }
            break;
        case '"':
            output[output_index++] = ch;
            if(i > 0 && app->text_box_store[i - 1] != '\\') {
                in_string = !in_string;
            }
            break;
        default:
            output[output_index++] = ch;
            break;
        }
    }

    output[output_index] = '\0'; // Null-terminate the output string
}
