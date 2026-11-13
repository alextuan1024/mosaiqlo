# app_time

Owns the single Wi-Fi station, SNTP client, and a boot-lifetime worker. `app_time_start()` initializes Wi-Fi even with no credentials. UI commands enter a one-element queue without blocking; scans and driver/storage calls execute on the worker, while snapshots use a short mutex. Network events only update an atomic IP-ready flag. Never run another Wi-Fi owner concurrently.

`wifi_request_scan()` scans nearby named access points, deduplicates SSIDs, and returns up to 32 results ordered by the driver. `wifi_request_connect(ssid, password)` accepts a copied password, an empty string for open networks, or NULL for a stored profile. An IP address and live matching association are required before success and credential persistence. Failed connections time out after 20 seconds. Previously successful networks reconnect after a five-second delay. Manual unsuccessful credentials do not overwrite saved ones.

The application stores the 16 most recently used successful networks in the default NVS partition, namespace `clock_wifi`, key `profiles`. The last successful network is selected at boot. Existing Vibe Mode `wifi_ssid` and `wifi_password` are imported through the read-only public API; shared settings remain unchanged. Profiles contain passwords; no passwords are logged or exposed in snapshots. NVS blob sizes, counts, string termination, and credential lengths are validated on load.

Open and WPA/WPA2/WPA3 Personal networks are supported. Enterprise authentication, WEP, hidden-SSID entry, and captive-portal login are not implemented. Unsupported security is labeled in the list.

SNTP uses `pool.ntp.org` with the build's one-hour interval. Clock time continues through outages; cold startup needs a successful sync. Initialization failures unwind resources. Successful services live until reboot. The worker owns driver buffers and credentials on the heap, with a 4096-byte task stack.
