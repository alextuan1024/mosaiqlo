#pragma once
/* Boot initialization follows panel creation. Runtime calls belong to the UI task. */
int brightness_initialize(void);
int brightness_get_percent(void);
/* Apply immediately; valid range 5..100. Returns zero on success. */
int brightness_set_percent(int percent);
/* Persist the applied value, normally on slider release or Back. */
int brightness_save(void);
