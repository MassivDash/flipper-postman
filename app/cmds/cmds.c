#include "cmds.h"
#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_serial.h>

bool postman_send_command_and_check_response(Postman *postman, CommandType cmd,
                                             const char *argument) {
  char command[256];
  switch (cmd) {
  case CMD_SET_SSID:
    snprintf(command, sizeof(command), "SET_SSID %s", argument);
    break;
  case CMD_SET_PASSWORD:
    snprintf(command, sizeof(command), "SET_PASSWORD %s", argument);
    break;
  case CMD_ACTIVATE_WIFI:
    snprintf(command, sizeof(command), "ACTIVATE_WIFI");
    break;
  case CMD_DISCONNECT_WIFI:
    snprintf(command, sizeof(command), "DISCONNECT_WIFI");
    break;
  case CMD_LIST_WIFI:
    snprintf(command, sizeof(command), "LIST_WIFI");
    break;
  case CMD_GET:
    snprintf(command, sizeof(command), "GET %s", argument);
    break;
  case CMD_GET_STREAM:
    snprintf(command, sizeof(command), "GET_STREAM %s", argument);
    break;
  case CMD_POST:
    snprintf(command, sizeof(command), "POST %s", argument);
    break;
  case CMD_BUILD_HTTP_METHOD:
    snprintf(command, sizeof(command), "BUILD_HTTP_METHOD %s", argument);
    break;
  case CMD_BUILD_HTTP_URL:
    snprintf(command, sizeof(command), "BUILD_HTTP_URL %s", argument);
    break;
  case CMD_BUILD_HTTP_HEADER:
    snprintf(command, sizeof(command), "BUILD_HTTP_HEADER %s", argument);
    break;
  case CMD_BUILD_HTTP_PAYLOAD:
    snprintf(command, sizeof(command), "BUILD_HTTP_PAYLOAD %s", argument);
    break;
  case CMD_REMOVE_HTTP_HEADER:
    snprintf(command, sizeof(command), "REMOVE_HTTP_HEADER %s", argument);
    break;
  case CMD_RESET_HTTP_CONFIG:
    snprintf(command, sizeof(command), "RESET_HTTP_CONFIG");
    break;
  case CMD_BUILD_HTTP_SHOW_RESPONSE_HEADERS:
    snprintf(command, sizeof(command), "BUILD_HTTP_SHOW_RESPONSE_HEADERS %s",
             argument);
    break;
  case CMD_BUILD_HTTP_IMPLEMENTATION:
    snprintf(command, sizeof(command), "BUILD_HTTP_IMPLEMENTATION %s",
             argument);
    break;
  case CMD_EXECUTE_HTTP_CALL:
    snprintf(command, sizeof(command), "EXECUTE_HTTP_CALL");
    break;
  case CMD_CONNECT:
    snprintf(command, sizeof(command), "CONNECT %s", argument);
    break;
  case CMD_HELP:
    snprintf(command, sizeof(command), "HELP");
    break;
  default:
    FURI_LOG_E("Postman", "Unknown command.");
    return false;
  }

  if (!postman_send_data(postman, command)) {
    FURI_LOG_E("Postman", "Failed to send command.");
    return false;
  }

  furi_delay_ms(100);

  if (postman->last_response) {
    FURI_LOG_I("Postman", "Response: %s", postman->last_response);
    return true;
  } else {
    FURI_LOG_E("Postman", "No response received.");
    return false;
  }
}

void setSSIDCommand(Postman *postman, const char *ssid) {
  postman_send_command_and_check_response(postman, CMD_SET_SSID, ssid);
}

void setPasswordCommand(Postman *postman, const char *password) {
  postman_send_command_and_check_response(postman, CMD_SET_PASSWORD, password);
}

void activateWiFiCommand(Postman *postman) {
  postman_send_command_and_check_response(postman, CMD_ACTIVATE_WIFI, NULL);
}

void disconnectWiFiCommand(Postman *postman) {
  postman_send_command_and_check_response(postman, CMD_DISCONNECT_WIFI, NULL);
}

void listWiFiCommand(Postman *postman) {
  postman_send_command_and_check_response(postman, CMD_LIST_WIFI, NULL);
}

void getCommand(Postman *postman, const char *url) {
  postman_send_command_and_check_response(postman, CMD_GET, url);
}

void getStreamCommand(Postman *postman, const char *url) {
  postman_send_command_and_check_response(postman, CMD_GET_STREAM, url);
}

void postCommand(Postman *postman, const char *url, const char *json_payload) {
  char argument[512];
  snprintf(argument, sizeof(argument), "%s %s", url, json_payload);
  postman_send_command_and_check_response(postman, CMD_POST, argument);
}

void buildHttpMethodCommand(Postman *postman, const char *method) {
  postman_send_command_and_check_response(postman, CMD_BUILD_HTTP_METHOD,
                                          method);
}

void buildHttpUrlCommand(Postman *postman, const char *url) {
  postman_send_command_and_check_response(postman, CMD_BUILD_HTTP_URL, url);
}

void buildHttpHeaderCommand(Postman *postman, const char *header) {
  postman_send_command_and_check_response(postman, CMD_BUILD_HTTP_HEADER,
                                          header);
}

void buildHttpPayloadCommand(Postman *postman, const char *payload) {
  postman_send_command_and_check_response(postman, CMD_BUILD_HTTP_PAYLOAD,
                                          payload);
}

void removeHttpHeaderCommand(Postman *postman, const char *header) {
  postman_send_command_and_check_response(postman, CMD_REMOVE_HTTP_HEADER,
                                          header);
}

void resetHttpConfigCommand(Postman *postman) {
  postman_send_command_and_check_response(postman, CMD_RESET_HTTP_CONFIG, NULL);
}

void buildHttpShowResponseHeadersCommand(Postman *postman, const char *show) {
  postman_send_command_and_check_response(
      postman, CMD_BUILD_HTTP_SHOW_RESPONSE_HEADERS, show);
}

void buildHttpImplementationCommand(Postman *postman,
                                    const char *implementation) {
  postman_send_command_and_check_response(
      postman, CMD_BUILD_HTTP_IMPLEMENTATION, implementation);
}

void executeHttpCallCommand(Postman *postman) {
  postman_send_command_and_check_response(postman, CMD_EXECUTE_HTTP_CALL, NULL);
}

void connectCommand(Postman *postman, const char *ssid, const char *password) {
  char argument[512];
  snprintf(argument, sizeof(argument), "%s %s", ssid, password);
  postman_send_command_and_check_response(postman, CMD_CONNECT, argument);
}

void helpCommand(Postman *postman) {
  postman_send_command_and_check_response(postman, CMD_HELP, NULL);
}