#ifndef VIDEO_COMM_TRIM_H
#define VIDEO_COMM_TRIM_H

/*
 * Last commercial that already runs to EOF (clock padding) may start on a
 * short dramatic fade, not credits. Skip that lead-in when it is shorter
 * than a commercial break, not a strict ad length, and follows a long show.
 * Mid-show pods are not trimmed.
 * ponytail: decision only; caller moves the cut.
 */
static inline int eof_comm_skip_leadin(double lead_len, int strict,
                                       double prev_show_len, double min_comm,
                                       double min_show)
{
    return lead_len > 0.0 && lead_len < min_comm && strict < 2
           && prev_show_len > min_show;
}

#endif
