#pragma once
#include <stdint.h>
typedef struct {
    int candidate;
    uint64_t candidate_since_ms;
    uint64_t last_sample_ms;
    int rotation;
} orientation_t;
/* Acceleration in g, X toward screen right, Y toward screen top.
 * Returns the stable clockwise display rotation in degrees. */
int orientation_update(orientation_t *state, float x, float y, float z, uint64_t now_ms);
