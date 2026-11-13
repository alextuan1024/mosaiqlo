#pragma once
#include "esp_err.h"
/* Start the boot-lifetime Wi-Fi worker and SNTP. Missing credentials leave
 * scanning/provisioning available; initialization errors preserve the clock UI. */
esp_err_t app_time_start(void);
