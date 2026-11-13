#include "app_settings.h"
#include "wifi_service.h"
#include "brightness_service.h"
#define GSP_BUNDLE_ENABLE_RAW_IDS
#include "bundle_gsp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

typedef enum { PAGE_CLOCK, PAGE_SETTINGS, PAGE_WIFI, PAGE_PASSWORD, PAGE_BRIGHTNESS } settings_page_t;
struct app_settings {
    esp_gsp_handle_t ui;
    void *timer;
    esp_gsp_list_t list;
    settings_page_t page;
    wifi_snapshot_t network;
    char selected_ssid[33];
    unsigned rendered_revision;
    uint64_t next_poll_ms;
    char password[65];
    esp_gsp_err_t last_error;
};
/* GSP retains list callbacks until UI destruction; this singleton lives until reboot. */
static app_settings_t settings;
static void clear_password(app_settings_t *controller) {
    volatile char *p=controller->password;
    for (size_t i=0;i<sizeof(controller->password);++i) p[i]=0;
}
static uint64_t now_ms(void) {
    struct timespec now; clock_gettime(CLOCK_MONOTONIC,&now);
    return (uint64_t)now.tv_sec*1000+now.tv_nsec/1000000;
}
static void checked(app_settings_t *controller, esp_gsp_err_t error) {
    if (error!=ESP_GSP_OK) controller->last_error=error;
}
static void text(app_settings_t *controller,uint16_t bind,const char *value) {
    checked(controller,esp_gsp_set_text(controller->ui,bind,value));
}
static void show_page(app_settings_t *controller,settings_page_t page)
{
    if (controller->page==PAGE_PASSWORD) {
        clear_password(controller);
        text(controller,GSP_FLIP_BIND_PASSWORD_MASK,"");
    }
    controller->page=page;
    checked(controller,esp_gsp_set_visible(controller->ui,GSP_FLIP_BIND_CLOCK_PAGE_VISIBLE,page==PAGE_CLOCK));
    checked(controller,esp_gsp_set_visible(controller->ui,GSP_FLIP_BIND_SETTINGS_PAGE_VISIBLE,page!=PAGE_CLOCK));
    checked(controller,esp_gsp_set_visible(controller->ui,GSP_FLIP_BIND_MENU_VISIBLE,page==PAGE_SETTINGS));
    checked(controller,esp_gsp_set_visible(controller->ui,GSP_FLIP_BIND_NETWORKS_VISIBLE,page==PAGE_WIFI));
    checked(controller,esp_gsp_set_visible(controller->ui,GSP_FLIP_BIND_PASSWORD_VISIBLE,page==PAGE_PASSWORD));
    checked(controller,esp_gsp_set_visible(controller->ui,GSP_FLIP_BIND_BRIGHTNESS_VISIBLE,page==PAGE_BRIGHTNESS));
    text(controller,GSP_FLIP_BIND_HEADING,page==PAGE_BRIGHTNESS ? "Brightness" : page==PAGE_SETTINGS ? "Settings" : page==PAGE_PASSWORD ? "Password" : "Wi-Fi");
    if (page==PAGE_BRIGHTNESS) {
        char value[12];
        snprintf(value,sizeof(value),"%d%%",brightness_get_percent());
        text(controller,GSP_FLIP_BIND_BRIGHTNESS_VALUE,value);
        text(controller,GSP_FLIP_BIND_BRIGHTNESS_HINT,"Drag to adjust. Saved automatically.");
        checked(controller,esp_gsp_set_value(controller->ui,GSP_FLIP_BIND_BRIGHTNESS_SLIDER,brightness_get_percent()));
    }
    if (page==PAGE_PASSWORD) {
        clear_password(controller);
        text(controller,GSP_FLIP_BIND_SELECTED_SSID,controller->selected_ssid);
        text(controller,GSP_FLIP_BIND_PASSWORD_MASK,"Enter password");
        text(controller,GSP_FLIP_BIND_PASSWORD_HINT,"8-63 characters, or 64 hex digits");

    }
}
static esp_gsp_err_t bind_network(esp_gsp_handle_t ui,esp_gsp_row_t row,uint32_t index,void *context)
{
    app_settings_t *controller=context;
    if (index>=controller->network.count) return esp_gsp_row_text(ui,row,"");
    const wifi_access_point_t *ap=&controller->network.access_points[index];
    char label[100];
    const char *status=!ap->supported ? "Unsupported security" :
        controller->network.state==WIFI_CONNECTED && !strcmp(ap->ssid,controller->network.ssid) ? "Connected" :
        ap->saved ? "Saved" : ap->secured ? "Password required" : "Open";
    snprintf(label,sizeof(label),"%s\n%s   %d dBm",ap->ssid,status,ap->rssi);
    return esp_gsp_row_text(ui,row,label);
}
static void request_scan(app_settings_t *controller)
{
    show_page(controller,PAGE_WIFI);
    if (wifi_request_scan()!=0) text(controller,GSP_FLIP_BIND_NETWORK_STATUS,"Wi-Fi busy. Tap Refresh to retry.");
    else text(controller,GSP_FLIP_BIND_NETWORK_STATUS,"Scanning...");
}
static void event(esp_gsp_handle_t ui,const esp_gsp_event_t *event,void *context)
{
    (void)ui;
    app_settings_t *controller=context;
    if (event->type!=ESP_GSP_EVENT_CALL) return;
    switch (event->action_id) {
    case GSP_FLIP_ACT_ID_OPEN_SETTINGS:
        if (controller->page==PAGE_CLOCK) show_page(controller,PAGE_SETTINGS);
        break;
    case GSP_FLIP_ACT_ID_OPEN_BRIGHTNESS:
        show_page(controller,PAGE_BRIGHTNESS);
        break;
    case GSP_FLIP_ACT_ID_BRIGHTNESS_CHANGE: {
        if (controller->page!=PAGE_BRIGHTNESS) break;
        int error=brightness_set_percent((int)event->arg);
        char value[12];
        snprintf(value,sizeof(value),"%d%%",brightness_get_percent());
        text(controller,GSP_FLIP_BIND_BRIGHTNESS_VALUE,value);
        if (error) {
            text(controller,GSP_FLIP_BIND_BRIGHTNESS_HINT,"Could not change brightness. Try again.");
            checked(controller,esp_gsp_set_value(ui,GSP_FLIP_BIND_BRIGHTNESS_SLIDER,brightness_get_percent()));
        } else text(controller,GSP_FLIP_BIND_BRIGHTNESS_HINT,"Drag to adjust. Saved automatically.");
        break;
    }
    case GSP_FLIP_ACT_ID_BRIGHTNESS_SAVE:
        if (controller->page==PAGE_BRIGHTNESS && brightness_save()!=0)
            text(controller,GSP_FLIP_BIND_BRIGHTNESS_HINT,"Could not save. Tap Back to retry.");
        break;
    case GSP_FLIP_ACT_ID_BACK:
        if (controller->page==PAGE_BRIGHTNESS && brightness_save()!=0) {
            text(controller,GSP_FLIP_BIND_BRIGHTNESS_HINT,"Could not save. Tap Back to retry.");
            break;
        }
        show_page(controller,controller->page==PAGE_SETTINGS ? PAGE_CLOCK : controller->page==PAGE_PASSWORD ? PAGE_WIFI : PAGE_SETTINGS);
        break;
    case GSP_FLIP_ACT_ID_OPEN_WIFI:
    case GSP_FLIP_ACT_ID_REFRESH:
        request_scan(controller); break;
    case GSP_FLIP_ACT_ID_CHOOSE_NETWORK: {
        if (controller->page!=PAGE_WIFI || event->item>=controller->network.count || controller->network.scanning) break;
        const wifi_access_point_t *ap=&controller->network.access_points[event->item];
        if (!ap->supported) { text(controller,GSP_FLIP_BIND_NETWORK_STATUS,"This security type is not supported."); break; }
        snprintf(controller->selected_ssid,sizeof(controller->selected_ssid),"%s",ap->ssid);
        bool retry=controller->network.state==WIFI_FAILED && !strcmp(ap->ssid,controller->network.ssid);
        if (ap->secured && (!ap->saved || retry)) show_page(controller,PAGE_PASSWORD);
        else if (wifi_request_connect(ap->ssid,ap->secured ? NULL : "")!=0)
            text(controller,GSP_FLIP_BIND_NETWORK_STATUS,"Wi-Fi busy. Try again.");
        else text(controller,GSP_FLIP_BIND_NETWORK_STATUS,"Connecting...");
        break;
    }
    case GSP_FLIP_ACT_ID_PASSWORD_KEYS_KEY: {
        if (controller->page!=PAGE_PASSWORD) break;
        if (event->arg==13) {
            if (strlen(controller->password)>=8 && wifi_request_connect(controller->selected_ssid,controller->password)==0) {
                show_page(controller,PAGE_WIFI);
                text(controller,GSP_FLIP_BIND_NETWORK_STATUS,"Connecting...");
            } else text(controller,GSP_FLIP_BIND_PASSWORD_HINT,"Check password length, then try OK again.");
        } else {
            size_t length=strlen(controller->password);
            if (event->arg==8 && length) controller->password[--length]=0;
            else if (event->arg>=32 && event->arg<=126 && length<64) {
                controller->password[length++]=(char)event->arg; controller->password[length]=0;
            }
            char mask[64];
            snprintf(mask,sizeof(mask),"%.*s  (%u)",(int)(length>20 ? 20 : length),"********************",(unsigned)length);
            text(controller,GSP_FLIP_BIND_PASSWORD_MASK,length ? mask : "Enter password");
        }
        break;
    }
    default: break;
    }
}
static void tick(esp_gsp_handle_t ui,void *context)
{
    app_settings_t *controller=context;
    uint64_t now=now_ms();
    if (now<controller->next_poll_ms) return;
    controller->next_poll_ms=now+200;
    if (wifi_read_snapshot(&controller->network)!=0) {
        if (controller->page==PAGE_WIFI) text(controller,GSP_FLIP_BIND_NETWORK_STATUS,"Wi-Fi unavailable. Restart to retry.");
        return;
    }
    if (controller->network.revision==controller->rendered_revision) return;
    controller->rendered_revision=controller->network.revision;
    char message[120];
    if (controller->network.scanning) snprintf(message,sizeof(message),"Scanning...");
    else if (controller->network.state==WIFI_CONNECTED) snprintf(message,sizeof(message),"%s: %s",controller->network.message,controller->network.ssid);
    else snprintf(message,sizeof(message),"%s",controller->network.message);
    text(controller,GSP_FLIP_BIND_NETWORK_STATUS,message);
    checked(controller,esp_gsp_list_set_total(ui,controller->list,controller->network.count));
    checked(controller,esp_gsp_list_refresh(ui,controller->list));
    if (!controller->network.scanning && !controller->network.count && controller->network.state==WIFI_IDLE)
        text(controller,GSP_FLIP_BIND_NETWORK_STATUS,"No networks found. Tap Refresh.");
}
esp_gsp_err_t app_settings_create(esp_gsp_handle_t ui,app_settings_t **out)
{
    if (!ui || !out) return ESP_GSP_ERR_INVALID_ARG;
    *out=NULL;
    if (settings.ui) return ESP_GSP_ERR_INVALID_STATE;
    app_settings_t *controller=&settings;
    controller->ui=ui; controller->rendered_revision=UINT32_MAX;
    controller->list=esp_gsp_list_bind_component(ui,GSP_FLIP_OBJ_KEY_NETWORKS,bind_network,controller);
    if (controller->list==ESP_GSP_LIST_NONE) { return ESP_GSP_ERR_NO_MEM; }
    checked(controller,esp_gsp_on_event(ui,event,controller));
    controller->timer=esp_gsp_timer_create(ui,40,tick,controller);
    if (!controller->timer) controller->last_error=ESP_GSP_ERR_NO_MEM;
    if (controller->last_error!=ESP_GSP_OK) {
        /* List retains its context until UI teardown; keep it valid on init failure. */
        esp_gsp_on_event(ui,NULL,NULL);
        return controller->last_error;
    }
    *out=controller;
    return ESP_GSP_OK;
}
void app_settings_destroy(app_settings_t *controller)
{
    if (!controller) return;
    esp_gsp_timer_delete(controller->ui,controller->timer);
    esp_gsp_on_event(controller->ui,NULL,NULL);
    clear_password(controller);
    /* List binder is framework-owned until UI destruction, so its context stays alive. */
}
