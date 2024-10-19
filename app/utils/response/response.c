#include <furi.h>
#include <furi_hal.h>
#include <string.h>
#include "../../app.h" // Include the app header to access app structure

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

    FuriString* furi_str = furi_string_alloc_set(app->text_box_store);
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
            furi_string_set(app->text_box_store, extracted_str);

            // Free the allocated FuriStrings
            furi_string_free(furi_str);
            furi_string_free(extracted_str);

            return true;
        } else {
            // If no end marker is found, extract till the end of the string
            FuriString* remaining_str = furi_string_alloc();
            furi_string_set_n(
                remaining_str, furi_str, start_pos, furi_string_size(furi_str) - start_pos);

            // Trim leading and trailing spaces
            furi_string_trim(remaining_str, " ");

            furi_string_set(app->text_box_store, remaining_str);

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
    FuriString* furi_str = furi_string_alloc_set(app->text_box_store);
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
    furi_string_set(app->text_box_store, cleaned_str);

    // Free the allocated FuriStrings
    furi_string_free(furi_str);
    furi_string_free(cleaned_str);
}

bool extract_response_stream(App* app) {
    const char* start_marker = "STREAM:";
    const char* end_marker = "STREAM_END";

    FuriString* furi_str = furi_string_alloc_set(app->text_box_store);
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
        furi_string_set(app->text_box_store, extracted_str);

        // Free the allocated FuriStrings
        furi_string_free(furi_str);
        furi_string_free(extracted_str);

        return true;
    }

    // Free the allocated FuriString
    furi_string_free(furi_str);

    return false; // No valid stream command found
}

bool extract_status_line(App* app, FuriString* status_line) {
    const char* status_marker = "STATUS: ";
    FuriString* furi_str = furi_string_alloc_set(app->text_box_store);
    size_t start_pos = 0;

    while((start_pos = furi_string_search_str(furi_str, status_marker, start_pos)) !=
          FURI_STRING_FAILURE) {
        start_pos += strlen(status_marker);

        // Ensure there are at least three characters after the status marker
        if(furi_string_size(furi_str) >= start_pos + 3) {
            // Copy the entire status line including the marker and the next three characters
            furi_string_set_n(
                status_line,
                furi_str,
                start_pos - strlen(status_marker),
                3 + strlen(status_marker));
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
    FuriString* furi_str = furi_string_alloc_set(app->text_box_store);
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
    FuriString* furi_str = furi_string_alloc_set(app->text_box_store);
    size_t start_pos = 0;

    while((start_pos = furi_string_search_str(furi_str, packet_marker, start_pos)) !=
          FURI_STRING_FAILURE) {
        start_pos += strlen(packet_marker);
        if(is_json(app)) {
            furi_string_free(furi_str);
            return true;
        }
    }

    // Check the whole string if no packet markers are found
    bool result = is_json(app);
    furi_string_free(furi_str);
    return result;
}

bool prettify_json(App* app, FuriString* output) {
    size_t indent = 0;
    bool in_string = false;

    for(size_t i = 0; i < furi_string_size(app->text_box_store); i++) {
        char ch = furi_string_get_char(app->text_box_store, i);

        switch(ch) {
        case '{':
        case '[':
            if(!in_string) {
                furi_string_push_back(output, ch);
                // Check for empty array or object
                if(furi_string_get_char(app->text_box_store, i + 1) == '}' ||
                   furi_string_get_char(app->text_box_store, i + 1) == ']') {
                    furi_string_push_back(output, furi_string_get_char(app->text_box_store, ++i));
                } else {
                    furi_string_push_back(output, '\n');
                    indent++;
                    for(size_t j = 0; j < indent; j++) {
                        furi_string_push_back(output, ' ');
                    }
                }
            } else {
                furi_string_push_back(output, ch);
            }
            break;
        case '}':
        case ']':
            if(!in_string) {
                furi_string_push_back(output, '\n');
                indent--;
                for(size_t j = 0; j < indent; j++) {
                    furi_string_push_back(output, ' ');
                }
                furi_string_push_back(output, ch);
            } else {
                furi_string_push_back(output, ch);
            }
            break;
        case ',':
            if(!in_string) {
                furi_string_push_back(output, ch);
                furi_string_push_back(output, '\n');
                for(size_t j = 0; j < indent; j++) {
                    furi_string_push_back(output, ' ');
                }
            } else {
                furi_string_push_back(output, ch);
            }
            break;
        case ':':
            if(!in_string) {
                furi_string_push_back(output, ch);
                furi_string_push_back(output, ' ');
            } else {
                furi_string_push_back(output, ch);
            }
            break;
        case '"':
            furi_string_push_back(output, ch);
            if(i > 0 && furi_string_get_char(app->text_box_store, i - 1) != '\\') {
                in_string = !in_string;
            }
            break;
        default:
            furi_string_push_back(output, ch);
            break;
        }
    }
    return true;
}
