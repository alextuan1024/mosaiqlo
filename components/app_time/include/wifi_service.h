#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define WIFI_RESULT_LIMIT 32
#define WIFI_SAVED_LIMIT 16

typedef enum { WIFI_IDLE, WIFI_CONNECTING, WIFI_CONNECTED, WIFI_FAILED } wifi_state_t;
typedef struct {
    char ssid[33];
    int rssi;
    bool secured;
    bool saved;
    bool supported;
} wifi_access_point_t;
typedef struct {
    wifi_state_t state;
    bool scanning;
    unsigned revision;
    size_t count;
    char ssid[33];
    char message[80];
    wifi_access_point_t access_points[WIFI_RESULT_LIMIT];
} wifi_snapshot_t;
/* Nonblocking commands. NULL password selects stored credentials. Never logs secrets. */
int wifi_request_scan(void);
int wifi_request_connect(const char *ssid, const char *password);
/* Copies a mutex-consistent snapshot; no driver or storage operations. */
int wifi_read_snapshot(wifi_snapshot_t *snapshot);
