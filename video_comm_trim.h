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

/*
 * delete_show_before_first_commercial used to require start_block==1, so two
 * leading show blocks (previous programme + sting) were kept. Treat any
 * leading show as junk when the first commercial starts before a show-length
 * segment and still finishes inside the start-padding window.
 */
static inline int leading_junk_before_first_comm(int start_block,
                                                 double first_comm_start,
                                                 double first_comm_end,
                                                 double min_show,
                                                 double pad_window)
{
    return start_block > 0
           && first_comm_start > 0.0 && first_comm_start < min_show
           && first_comm_end < pad_window;
}

#endif
