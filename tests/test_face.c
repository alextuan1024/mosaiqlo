#include <assert.h>
#include <stdio.h>
#include "flip_face.h"
int main(void) {
    flip_face_t face = {0};
    assert(flip_face_update(&face, 23 * 60 + 59, 1000));
    assert(face.previous_minute == 1439 && face.minute == 1439);
    assert(!flip_face_update(&face, 1439, 2000));
    assert(flip_face_update(&face, 0, 3000));
    assert(face.previous_minute == 1439 && face.minute == 0);
    assert(flip_face_update(&face, 0, 3300));
    assert(flip_face_progress(&face, 3300) > 0.49f);
    assert(flip_face_progress(&face, 3600) == 1.0f);
    assert(flip_face_update(&face, 0, 3600));
    assert(!flip_face_update(&face, 0, 3650));
    assert(!flip_face_update(&face, -1, 3700));
    assert(face.minute == 0);
    assert(flip_face_pixel(&face, 3650, 220, 100) == 0);
    unsigned lit = 0;
    for (int y = 0; y < 224; ++y)
        for (int x = 0; x < 216; ++x)
            if (flip_face_pixel(&face, 3650, x, y) > 100) ++lit;
    assert(lit > 3000);
    puts("PASS: startup, minute change, midnight, animation completion, invalid time, face pixels");
}
