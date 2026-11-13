#include "orientation.h"
#include <math.h>

#define STABLE_TIME_MS 400
#define MAX_SAMPLE_GAP_MS 250

int orientation_update(orientation_t *state, float x, float y, float z, uint64_t now_ms)
{
    float magnitude_squared = x*x + y*y + z*z;
    float horizontal = fabsf(x), vertical = fabsf(y);
    int candidate = -1;
    if (isfinite(magnitude_squared) && magnitude_squared >= 0.64f && magnitude_squared <= 1.44f &&
        fabsf(z) < 0.8f && fabsf(horizontal - vertical) > 0.2f) {
        if (horizontal >= 0.65f && horizontal > vertical) candidate = x > 0 ? 90 : 270;
        else if (vertical >= 0.65f) candidate = y > 0 ? 0 : 180;
    }
    if (candidate != state->candidate || now_ms - state->last_sample_ms > MAX_SAMPLE_GAP_MS) {
        state->candidate = candidate;
        state->candidate_since_ms = now_ms;
    }
    state->last_sample_ms = now_ms;
    if (candidate >= 0 && now_ms - state->candidate_since_ms >= STABLE_TIME_MS)
        state->rotation = candidate;
    return state->rotation;
}
