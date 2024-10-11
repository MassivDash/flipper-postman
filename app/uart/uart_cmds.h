#ifndef UART_CMDS_H
#define UART_CMDS_H

#include <furi.h>
#include <furi_hal.h>
#include <stddef.h>

typedef struct {
  const char *name;
  const char *description;
  void (*execute)(const char *argument);
} Command;

extern Command commands[];

void setSSIDCommand(const char *argument);
void setPasswordCommand(const char *argument);
void activateWiFiCommand(const char *argument);
void disconnectWiFiCommand(const char *argument);
void listWiFiCommand(const char *argument);
void getCommand(const char *argument);
void getStreamCommand(const char *argument);
void postCommand(const char *argument);
void buildHttpMethodCommand(const char *argument);
void buildHttpUrlCommand(const char *argument);
void buildHttpHeaderCommand(const char *argument);
void buildHttpPayloadCommand(const char *argument);
void removeHttpHeaderCommand(const char *argument);
void resetHttpConfigCommand(const char *argument);
void buildHttpShowResponseHeadersCommand(const char *argument);
void buildHttpImplementationCommand(const char *argument);
void executeHttpCallCommand(const char *argument);
void connectCommand(const char *argument);

#endif // UART_CMDS_H