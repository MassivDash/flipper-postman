#include "uart_cmds.h"
#include "uart.h"
#include <furi.h>
#include <string.h>

extern Uart *uart; // Assuming uart is defined elsewhere

Command commands[] = {
    {"SET_SSID", "SET ssid <ssid>", setSSIDCommand},
    {"SET_PASSWORD", "SET password <password>", setPasswordCommand},
    {"ACTIVATE_WIFI", "ACTIVATE_WIFI", activateWiFiCommand},
    {"DISCONNECT_WIFI", "DISCONNECT_WIFI", disconnectWiFiCommand},
    {"LIST_WIFI", "LIST_WIFI", listWiFiCommand},
    {"GET", "GET <url>", getCommand},
    {"GET_STREAM", "GET_STREAM <url>", getStreamCommand},
    {"POST", "POST <url> <json_payload>", postCommand},
    {"BUILD_HTTP_METHOD", "BUILD_HTTP_METHOD <method>", buildHttpMethodCommand},
    {"BUILD_HTTP_URL", "BUILD_HTTP_URL <url>", buildHttpUrlCommand},
    {"BUILD_HTTP_HEADER", "BUILD_HTTP_HEADER <header>", buildHttpHeaderCommand},
    {"BUILD_HTTP_PAYLOAD", "BUILD_HTTP_PAYLOAD <payload>",
     buildHttpPayloadCommand},
    {"REMOVE_HTTP_HEADER", "REMOVE_HTTP_HEADER <header>",
     removeHttpHeaderCommand},
    {"RESET_HTTP_CONFIG", "RESET_HTTP_CONFIG", resetHttpConfigCommand},
    {"BUILD_HTTP_SHOW_RESPONSE_HEADERS",
     "BUILD_HTTP_SHOW_RESPONSE_HEADERS <true/false>",
     buildHttpShowResponseHeadersCommand},
    {"BUILD_HTTP_IMPLEMENTATION", "BUILD_HTTP_IMPLEMENTATION <STREAM/CALL>",
     buildHttpImplementationCommand},
    {"EXECUTE_HTTP_CALL", "EXECUTE_HTTP_CALL", executeHttpCallCommand},
    {"CONNECT", "CONNECT <SSID> <password>", connectCommand}};

void setSSIDCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "WIFI_SSID: %s", argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void setPasswordCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "WIFI_PASSWORD: %s", argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void activateWiFiCommand(const char *argument) {
  UNUSED(argument);
  FURI_LOG_D("UART_CMDS", "WIFI_CONNECT: Connecting to WiFi...");
  // Send the command to the UART
  const char *command = "ACTIVATE_WIFI\n";
  uart_terminal_uart_tx(uart, (uint8_t *)command, strlen(command));
}

void disconnectWiFiCommand(const char *argument) {
  UNUSED(argument);
  FURI_LOG_D("UART_CMDS", "WIFI_DISCONNECT: Wifi disconnected");
  // Send the command to the UART
  const char *command = "DISCONNECT_WIFI\n";
  uart_terminal_uart_tx(uart, (uint8_t *)command, strlen(command));
}

void listWiFiCommand(const char *argument) {
  UNUSED(argument);
  FURI_LOG_D("UART_CMDS", "WIFI_LIST: <list>");
  // Send the command to the UART
  const char *command = "LIST_WIFI\n";
  uart_terminal_uart_tx(uart, (uint8_t *)command, strlen(command));
}

void getCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS",
             "GET: %s\nSTATUS: <number>\nRESPONSE:\n<response>\nRESPONSE_END",
             argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void getStreamCommand(const char *argument) {
  FURI_LOG_D(
      "UART_CMDS",
      "GET_STREAM: %s\nSTATUS: <number>\nSTREAM:\n<streamed data>\nSTREAM_END",
      argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void postCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS",
             "POST: %s\nPayload: <json_payload>\nSTATUS: "
             "<number>\nRESPONSE:\n<response>\nRESPONSE_END",
             argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void buildHttpMethodCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "HTTP_SET_METHOD: %s", argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void buildHttpUrlCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "HTTP_URL: %s", argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void buildHttpHeaderCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "HTTP_ADD_HEADER: %s", argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void buildHttpPayloadCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "HTTP_SET_PAYLOAD: %s", argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void removeHttpHeaderCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "HTTP_REMOVE_HEADER: %s", argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void resetHttpConfigCommand(const char *argument) {
  UNUSED(argument);
  FURI_LOG_D("UART_CMDS", "HTTP_CONFIG_REST: All configurations reset");
  // Send the command to the UART
  const char *command = "RESET_HTTP_CONFIG\n";
  uart_terminal_uart_tx(uart, (uint8_t *)command, strlen(command));
}

void buildHttpShowResponseHeadersCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "HTTP_BUILDER_SHOW_RESPONSE_HEADERS: %s", argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void buildHttpImplementationCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "HTTP_SET_IMPLEMENTATION: %s", argument);
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}

void executeHttpCallCommand(const char *argument) {
  UNUSED(argument);
  FURI_LOG_D("UART_CMDS", "EXECUTE_HTTP_CALL");
  // Send the command to the UART
  const char *command = "EXECUTE_HTTP_CALL\n";
  uart_terminal_uart_tx(uart, (uint8_t *)command, strlen(command));
}

void connectCommand(const char *argument) {
  FURI_LOG_D("UART_CMDS", "WIFI_SSID: <SSID>\nWIFI_PASSWORD: "
                          "<password>\nWIFI_CONNECT: Connecting to WiFi...");
  // Send the command to the UART
  uart_terminal_uart_tx(uart, (uint8_t *)argument, strlen(argument));
}
