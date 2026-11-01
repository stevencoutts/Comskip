#ifndef VIDEO_LUMA_H
#define VIDEO_LUMA_H

/*
 * Detection reads an 8-bit Y plane. IPTV HEVC is often 10-bit; scale
 * those frames to 8-bit before scoring. Hardware surfaces are retrieved
 * first, so treat them as not-yet-convertible here.
 * ponytail: depth>8 only; no codec-id branch.
 */
static inline int video_luma_needs_8bit(int depth, int hwaccel)
{
    if (hwaccel || depth <= 0)
        return 0;
    return depth > 8;
}

#endif
