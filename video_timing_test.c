#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "video_timing.h"

static int fails;

static void expect_near(const char *name, double got, double want)
{
    if (fabs(got - want) > 1e-9) {
        fprintf(stderr, "FAIL %s: got %.10f want %.10f\n", name, got, want);
        fails++;
    }
}

int main(void)
{
    /* PAL MPEG-2: 25 fps frames, r_frame_rate is field rate 50. */
    expect_near("pal-mpeg2", video_tick_duration(0.04, 50.0), 0.02);
    /* NTSC 30000/1001 with 60000/1001 fields. */
    expect_near("ntsc-mpeg2", video_tick_duration(1001.0 / 30000.0, 60000.0 / 1001.0),
                1001.0 / 60000.0);
    /* Progressive: r_frame_rate matches frame rate. */
    expect_near("progressive", video_tick_duration(0.04, 25.0), 0.04);
    expect_near("mpeg1", video_tick_duration(1.0 / 23.976, 23.976), 1.0 / 23.976);
    /* Missing r_frame_rate must not invent field ticks. */
    expect_near("no-r", video_tick_duration(0.04, 0.0), 0.04);
    expect_near("zero-delay", video_tick_duration(0.0, 50.0), 0.0);

    if (fails) {
        fprintf(stderr, "%d failed\n", fails);
        return 1;
    }
    printf("video_timing_test: ok\n");
    return 0;
}
