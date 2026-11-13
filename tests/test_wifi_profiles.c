#include "wifi_profiles.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static wifi_profiles_t profiles;
int main(void) {
    assert(wifi_credentials_valid("Cafe", ""));
    assert(wifi_credentials_valid("Home", "password"));
    assert(!wifi_credentials_valid("", "password"));
    assert(!wifi_credentials_valid("Home", "short"));
    assert(!wifi_credentials_valid("Home", "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"));
    assert(wifi_credentials_valid("Home", "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"));
    wifi_profile_remember(&profiles,"Home","password");
    wifi_profile_remember(&profiles,"Cafe","");
    assert(wifi_profile_find(&profiles,"Home") == 1);
    assert(wifi_profile_find(&profiles,"Unknown") == -1);
    wifi_profile_remember(&profiles,"Home","updatedpass");
    assert(profiles.count == 2 && !strcmp(profiles.entries[0].password,"updatedpass"));
    for (int i=0;i<20;++i) { char ssid[20]; snprintf(ssid,sizeof(ssid),"Network%d",i); wifi_profile_remember(&profiles,ssid,""); }
    assert(profiles.count == WIFI_SAVED_LIMIT && wifi_profile_find(&profiles,"Home")==-1);
    puts("PASS: credentials, saved lookup, replacement, bounded recent network storage");
}
