#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "orientation.h"
int main(void)
{
    orientation_t state = {0};
    const float poses[][3] = {{1,0,0},{0,-1,0},{-1,0,0},{0,1,0}};
    const int rotations[] = {90,180,270,0};
    uint64_t time = 100;
    for (int pose = 0; pose < 4; ++pose) {
        int previous = state.rotation;
        for (int sample = 0; sample < 5; ++sample, time += 100) {
            int rotation = orientation_update(&state, poses[pose][0], poses[pose][1], 0, time);
            assert(rotation == (sample < 4 ? previous : rotations[pose]));
        }
    }
    /* Flat, diagonal, shaking, and invalid samples must cancel a pending turn. */
    const float invalid[][3] = {{0,0,1},{0.707f,0.707f,0},{2,0,0},{NAN,0,0}};
    for (int i = 0; i < 4; ++i) {
        orientation_update(&state, 1,0,0,time); time += 100;
        assert(orientation_update(&state,invalid[i][0],invalid[i][1],invalid[i][2],time)==0);
        time += 100;
        assert(orientation_update(&state,1,0,0,time)==0);
        time += 100;
    }
    /* A sensor outage must not count as stable time. */
    time += 1000;
    assert(orientation_update(&state,1,0,0,time)==0);
    puts("PASS: four orientations, debounce, flat/diagonal/shake rejection, invalid samples, sensor gaps");
}
