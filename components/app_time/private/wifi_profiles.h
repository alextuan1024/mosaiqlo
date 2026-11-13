#pragma once
#include "wifi_service.h"
typedef struct { char ssid[33]; char password[65]; } wifi_profile_t;
typedef struct { unsigned count; wifi_profile_t entries[WIFI_SAVED_LIMIT]; } wifi_profiles_t;
int wifi_profile_find(const wifi_profiles_t *profiles, const char *ssid);
bool wifi_credentials_valid(const char *ssid, const char *password);
void wifi_profile_remember(wifi_profiles_t *profiles, const char *ssid, const char *password);
