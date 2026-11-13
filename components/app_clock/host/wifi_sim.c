#include <stdio.h>

#include "wifi_profiles.h"
#include <string.h>
/* Simulator-only network service; the UI/controller is shared with the device. */
static wifi_snapshot_t snapshot={.revision=1,.message="Choose a network"};
int wifi_request_scan(void) {
    snapshot.count=4;
    snapshot.access_points[0]=(wifi_access_point_t){"Saved Home",-35,true,true,true};
    snapshot.access_points[1]=(wifi_access_point_t){"New Network",-45,true,false,true};
    snapshot.access_points[2]=(wifi_access_point_t){"Open Cafe",-55,false,false,true};
    snapshot.access_points[3]=(wifi_access_point_t){"Enterprise",-65,true,false,false};
    ++snapshot.revision;return 0;
}
int wifi_request_connect(const char *ssid,const char *password) {
    if (password && !wifi_credentials_valid(ssid,password)) return 1;
    if (password && password[0] && strcmp(password,"password") && strcmp(password,"wrongpass")) return 1;
    snprintf(snapshot.ssid,sizeof(snapshot.ssid),"%s",ssid);
    snapshot.state=password && !strcmp(password,"wrongpass") ? WIFI_FAILED : WIFI_CONNECTED;
    snprintf(snapshot.message,sizeof(snapshot.message),"%s",snapshot.state==WIFI_FAILED ? "Connection failed. Check password and retry." : "Connected");
    if (snapshot.state==WIFI_CONNECTED)
        for (size_t i=0;i<snapshot.count;++i) if (!strcmp(snapshot.access_points[i].ssid,ssid)) snapshot.access_points[i].saved=true;
    fprintf(stderr,"wifi_sim: %s via %s\n",snapshot.state==WIFI_FAILED ? "failed" : "connected",password==NULL ? "saved" : password[0] ? "password" : "open");
    ++snapshot.revision;return 0;
}
int wifi_read_snapshot(wifi_snapshot_t *out) { *out=snapshot;return 0; }
