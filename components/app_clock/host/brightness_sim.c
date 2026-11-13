#include "brightness_service.h"
#include <stdio.h>
static int brightness_percent=40;
int brightness_get_percent(void) { return brightness_percent; }
int brightness_initialize(void) { return 0; }
int brightness_set_percent(int percent) {
    if (percent<5 || percent>100) return 1;
    brightness_percent=percent;
    fprintf(stderr,"brightness_sim: applied %d\n",percent);
    return 0;
}
int brightness_save(void) {
    fprintf(stderr,"brightness_sim: saved %d\n",brightness_percent);
    return 0;
}
