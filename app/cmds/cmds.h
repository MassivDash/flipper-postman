#ifndef CMDS_H
#define CMDS_H

#include "../uart/uart.h"

// Command types
typedef enum {
  CMD_SET_SSID,
  CMD_SET_PASSWORD,
  CMD_ACTIVATE_WIFI,
  CMD_DISCONNECT_WIFI,
  CMD_LIST_WIFI,
  CMD_GET,
  CMD_GET_STREAM,
  CMD_POST,
  CMD_BUILD_HTTP_METHOD,
  CMD_BUILD_HTTP_URL,
  CMD_BUILD_HTTP_HEADER,
  CMD_BUILD_HTTP_PAYLOAD,
  CMD_REMOVE_HTTP_HEADER,
  CMD_RESET_HTTP_CONFIG,
  CMD_BUILD_HTTP_SHOW_RESPONSE_HEADERS,
  CMD_BUILD_HTTP_IMPLEMENTATION,
  CMD_EXECUTE_HTTP_CALL,
  CMD_CONNECT,
  CMD_HELP,
  CMD_UNKNOWN
} CommandType;

// Response types
typedef enum {
  RESP_WIFI_CONNECTED,
  RESP_WIFI_DISCONNECTED,
  RESP_WIFI_SSID,
  RESP_WIFI_PASSWORD,
  RESP_WIFI_LIST,
  RESP_HTTP_GET,
  RESP_HTTP_GET_STREAM,
  RESP_HTTP_POST,
  RESP_HTTP_METHOD,
  RESP_HTTP_URL,
  RESP_HTTP_HEADER,
  RESP_HTTP_PAYLOAD,
  RESP_HTTP_REMOVE_HEADER,
  RESP_HTTP_RESET_CONFIG,
  RESP_HTTP_SHOW_RESPONSE_HEADERS,
  RESP_HTTP_IMPLEMENTATION,
  RESP_HTTP_EXECUTE_CALL,
  RESP_HELP,
  RESP_UNKNOWN
} ResponseType;

// Function declarations
bool postman_send_command_and_check_response(Postman *postman, CommandType cmd,
                                             const char *argument);
void setSSIDCommand(Postman *postman, const char *ssid);
void setPasswordCommand(Postman *postman, const char *password);
void activateWiFiCommand(Postman *postman);
void disconnectWiFiCommand(Postman *postman);
void listWiFiCommand(Postman *postman);
void getCommand(Postman *postman, const char *url);
void getStreamCommand(Postman *postman, const char *url);
void postCommand(Postman *postman, const char *url, const char *json_payload);
void buildHttpMethodCommand(Postman *postman, const char *method);
void buildHttpUrlCommand(Postman *postman, const char *url);
void buildHttpHeaderCommand(Postman *postman, const char *header);
void buildHttpPayloadCommand(Postman *postman, const char *payload);
void removeHttpHeaderCommand(Postman *postman, const char *header);
void resetHttpConfigCommand(Postman *postman);
void buildHttpShowResponseHeadersCommand(Postman *postman, const char *show);
void buildHttpImplementationCommand(Postman *postman,
                                    const char *implementation);
void executeHttpCallCommand(Postman *postman);
void connectCommand(Postman *postman, const char *ssid, const char *password);
void helpCommand(Postman *postman);

#endif // CMDS_H