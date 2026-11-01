#ifndef VIDEO_TIMING_H
#define VIDEO_TIMING_H

/*
 * Reconstruct the old AVCodecContext.ticks_per_frame unit (removed in FFmpeg 8).
 * MPEG-2/H.264 often advertise r_frame_rate at field rate (~2x), so PTS step
 * checks compared against n * frame_delay / 2, not n * frame_delay.
 * ponytail: ticks=2 iff r_fps > 1.5 * frame_fps; revisit if a codec reports
 * field-rate r_frame_rate without that 2x ratio.
 */
static inline double video_tick_duration(double frame_delay, double r_fps)
{
    if (frame_delay > 0.0 && r_fps > (1.0 / frame_delay) * 1.5)
        return frame_delay * 0.5;
    return frame_delay;
}

static inline int video_rate_ok(int num, int den)
{
    return num > 0 && den > 0;
}

#endif
