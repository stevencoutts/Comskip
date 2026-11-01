#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "video_timing.h"
#include "video_luma.h"
#include "video_comm_trim.h"

static int fails;

static void expect_near(const char *name, double got, double want)
{
    if (fabs(got - want) > 1e-9) {
        fprintf(stderr, "FAIL %s: got %.10f want %.10f\n", name, got, want);
        fails++;
    }
}

static void expect_int(const char *name, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s: got %d want %d\n", name, got, want);
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

    expect_int("8bit", video_luma_needs_8bit(8, 0), 0);
    expect_int("10bit", video_luma_needs_8bit(10, 0), 1);
    expect_int("12bit", video_luma_needs_8bit(12, 0), 1);
    expect_int("hw-10bit", video_luma_needs_8bit(10, 1), 0);
    expect_int("unknown", video_luma_needs_8bit(0, 0), 0);

    expect_int("eof-skip", eof_comm_skip_leadin(16.44, 0, 161.0, 20.0, 125.0), 1);
    expect_int("eof-strict", eof_comm_skip_leadin(16.44, 2, 161.0, 20.0, 125.0), 0);
    expect_int("eof-long-lead", eof_comm_skip_leadin(30.0, 0, 161.0, 20.0, 125.0), 0);
    expect_int("eof-no-show", eof_comm_skip_leadin(16.44, 0, 12.0, 20.0, 125.0), 0);

    expect_int("preroll", leading_junk_before_first_comm(2, 58.0, 83.0, 125.0, 840.0), 1);
    expect_int("no-preroll-late", leading_junk_before_first_comm(19, 1201.0, 1443.0, 125.0, 840.0), 0);
    expect_int("no-preroll-midbreak", leading_junk_before_first_comm(1, 480.0, 720.0, 125.0, 840.0), 0);
    expect_int("no-preroll-block0", leading_junk_before_first_comm(0, 58.0, 83.0, 125.0, 840.0), 0);

    if (fails) {
        fprintf(stderr, "%d failed\n", fails);
        return 1;
    }
    printf("video_timing_test: ok\n");
    return 0;
}
