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

    expect_int("ident-before", short_show_before_commercial(2.60, 1.00, 1.05, 7.0), 1);
    expect_int("ident-quiet", short_show_before_commercial(2.60, 0.50, 1.05, 7.0), 1);
    expect_int("show-scene", short_show_before_commercial(942.0, 0.06, 1.05, 7.0), 0);
    expect_int("already-commercial", short_show_before_commercial(2.60, 15.0, 1.05, 7.0), 0);
    expect_int("ident-disabled", short_show_before_commercial(2.60, 1.00, 1.05, 0.0), 0);
    expect_int("ident-at-limit", short_show_before_commercial(7.0, 1.00, 1.05, 7.0), 0);

    /* A few frames of black is a boundary. Minutes of cuts is the pod. */
    expect_int("cluster-separator", cut_cluster_is_content(40, 25.0, 125.0), 0);
    expect_int("cluster-doghouse", cut_cluster_is_content(22, 50.0, 125.0), 0);
    expect_int("cluster-pod", cut_cluster_is_content(7832, 25.0, 125.0), 1);
    expect_int("cluster-zero", cut_cluster_is_content(0, 25.0, 125.0), 0);

    /* Stargate's 11s PTS glitch stays negative. A long backwards span uses frames. */
    expect_near("len-glitch", block_length_seconds(11.16, 565.88, 14430, 14148, 25.0, 125.0),
                11.16 - 565.88);
    expect_near("len-jump", block_length_seconds(31.20, 82.52, 26267, 2064, 25.0, 125.0),
                (26267 - 2064) / 25.0);
    expect_near("len-positive", block_length_seconds(924.04, 598.00, 48588, 40437, 25.0, 125.0),
                924.04 - 598.00);

    /* HIGNFY pods sit between longer shows. Stargate/Dog House blocks do not. */
    expect_int("pod-break2", long_pod_between_shows(326.04, 374.92, 857.64, 125.0, 600.0), 1);
    expect_int("pod-break3", long_pod_between_shows(272.88, 857.64, 1127.92, 125.0, 600.0), 1);
    expect_int("show-between-pods", long_pod_between_shows(374.92, 326.04, 857.64, 125.0, 600.0), 0);
    expect_int("eof-show", long_pod_between_shows(1127.92, 857.64, 0.0, 125.0, 600.0), 0);
    expect_int("stargate-mid", long_pod_between_shows(264.96, 469.56, 225.92, 125.0, 600.0), 0);
    expect_int("stargate-eof", long_pod_between_shows(317.76, 225.92, 0.0, 125.0, 600.0), 0);
    expect_int("stargate-long", long_pod_between_shows(942.44, 550.68, 469.56, 125.0, 600.0), 0);
    expect_int("doghouse-show", long_pod_between_shows(526.44, 140.26, 39.0, 125.0, 600.0), 0);
    expect_int("doghouse-eof", long_pod_between_shows(805.78, 168.16, 0.0, 125.0, 600.0), 0);
    expect_int("over-max-break", long_pod_between_shows(700.0, 800.0, 900.0, 125.0, 600.0), 0);

    /* HIGNFY title card is a 6s letterbox at 2:22. Stargate's first-frame
     * glitch and its 9:13 ident are not the start of the programme. */
    expect_int("title-hignfy", opening_title_card(141.80, 6.00, 2.00, 831.0, 1.78, 1.78,
                                                  4.0, 20.0, 125.0, 250.0, 0.08), 1);
    expect_int("title-glitch", opening_title_card(0.04, 1.16, 2.72, 552.0, 1.78, 1.78,
                                                  4.0, 20.0, 125.0, 250.0, 0.08), 0);
    expect_int("title-midshow", opening_title_card(553.0, 9.96, 2.72, 200.0, 1.78, 1.78,
                                                   4.0, 20.0, 125.0, 250.0, 0.08), 0);
    expect_int("title-doghouse", opening_title_card(827.0, 30.0, 2.00, 1277.0, 1.78, 1.78,
                                                    4.0, 20.0, 125.0, 250.0, 0.08), 0);
    expect_int("junction", bright_break_after_dim(176, 30, 160, 100), 1);
    expect_int("junction-flash", bright_break_after_dim(176, 30, 40, 100), 0);
    expect_int("junction-show", bright_break_after_dim(90, 70, 85, 100), 0);

    /* Break tail scored as show, and the dark return scored as a short pod. */
    expect_int("tail-bbt", break_tail_then_false_return(352.84, 65.12, 120.60, 346.56, 1, 125.0, 20.0), 1);
    expect_int("tail-real-second", break_tail_then_false_return(352.84, 65.12, 180.0, 346.56, 1, 125.0, 20.0), 0);
    expect_int("tail-strict", break_tail_then_false_return(352.84, 65.12, 120.60, 346.56, 2, 125.0, 20.0), 0);
    expect_int("tail-long-gap", break_tail_then_false_return(244.0, 469.0, 90.0, 400.0, 0, 125.0, 20.0), 0);
    expect_int("tail-blip", break_tail_then_false_return(8.0, 40.0, 60.0, 400.0, 0, 125.0, 20.0), 0);

    /* Credits then the next episode. A cut-heavy pod, Stargate's EOF, and a
     * long following break are not that shape. */
    expect_int("next-bbt", credits_then_next_show(261.80, 1, 562.0, 3.60, 125.0, 20.0), 1);
    expect_int("next-no-tail", credits_then_next_show(261.80, 1, 562.0, 0.0, 125.0, 20.0), 1);
    expect_int("next-cutty", credits_then_next_show(323.0, 0, 1128.0, 0.0, 125.0, 20.0), 0);
    expect_int("next-stargate", credits_then_next_show(704.60, 1, 0.0, 0.0, 125.0, 20.0), 0);
    expect_int("next-another-break", credits_then_next_show(244.0, 1, 469.0, 244.0, 125.0, 20.0), 0);

    expect_near("span-pod", commercial_span_seconds(223.00 - 894.04, 348.40), 348.40);
    expect_near("span-glitch", commercial_span_seconds(255.68 - 550.72, -295.64), 255.68 - 550.72);
    expect_near("span-positive", commercial_span_seconds(326.04, 326.04), 326.04);

    if (fails) {
        fprintf(stderr, "%d failed\n", fails);
        return 1;
    }
    printf("video_timing_test: ok\n");
    return 0;
}
