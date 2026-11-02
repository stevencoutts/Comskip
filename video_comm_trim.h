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
 * A show block immediately before a commercial, shorter than
 * delete_block_after_commercial. That is the break ident: the cut should
 * start there, so the ident and the first ad frames are inside the break.
 * A real show scene is longer than the limit and stays.
 */
static inline int short_show_before_commercial(double lead_len, double lead_score,
                                               double threshold, double max_lead)
{
    return max_lead > 0.0 && lead_len > 0.0 && lead_len < max_lead
           && lead_score < threshold;
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

/*
 * A run of cut markers longer than a show segment is the ad pod itself.
 * A real black-frame boundary is a few frames. Credits-to-EOF is not
 * handled here: the caller keeps a cluster that already reaches EOF.
 */
static inline int cut_cluster_is_content(long frame_span, double fps, double min_show)
{
    if (!(fps > 0.0) || frame_span <= 0)
        return 0;
    return (double)frame_span / fps > min_show;
}

/*
 * PTS ran backwards across this block. A long frame span is the playback
 * length. A short span is a glitch (keep the negative PTS length so the
 * existing too-short rejection still drops it).
 */
static inline double block_length_seconds(double pts_end, double pts_start,
                                          long f_end, long f_start, double fps,
                                          double min_span)
{
    double pts = pts_end - pts_start;
    double frames;

    if (!(fps > 0.0))
        return pts;
    frames = (double)(f_end - f_start) / fps;
    if (pts < 0.0 && frames > min_span)
        return frames;
    return pts;
}

/*
 * A block longer than two show-minimums, but shorter than both neighbouring
 * show blocks and shorter than the longest allowed break, is a pod of spots
 * sitting between programme segments. A credits-to-EOF block has no following
 * show, so it stays a show. The caller skips blocks shorter than a show
 * when it picks the neighbours.
 */
static inline int long_pod_between_shows(double len, double before, double after,
                                         double min_show, double max_break)
{
    return len > 2.0 * min_show && len < max_break
           && before > len && after > len;
}

/*
 * A commercial that crosses a backwards PTS jump has a negative PTS span.
 * The block lengths already use the frame clock for a long jump. Use that
 * sum when it is positive. A short glitch still sums negative and stays
 * negative, so the too-short rejection still drops it.
 */
static inline double commercial_span_seconds(double pts_len, double block_len_sum)
{
    if (pts_len < 0.0 && block_len_sum > 0.0)
        return block_len_sum;
    return pts_len;
}

/*
 * A short non-dominant aspect segment early in the recording, followed by
 * a show-length run of the dominant aspect, is the title card. Everything
 * before it is the previous programme. A one-frame detector glitch and a
 * letterbox later than two show-lengths (a mid-episode ident) are not.
 */
static inline int opening_title_card(double start_sec, double len, double ar,
                                     double next_len, double next_ar, double dominant,
                                     double min_blip, double max_blip, double min_show,
                                     double window, double ar_tol)
{
    double gap = ar - dominant;
    double next_gap = next_ar - dominant;

    if (gap < 0.0)
        gap = -gap;
    if (next_gap < 0.0)
        next_gap = -next_gap;
    return start_sec > 1.0 && start_sec < window
           && len >= min_blip && len < max_blip
           && gap > ar_tol
           && next_len > min_show
           && next_gap <= ar_tol;
}

/*
 * Credits are a dim hold. The junction is the frame that steps well above
 * that hold and stays there, rather than a one-frame flash.
 */
static inline int bright_break_after_dim(int now, int prev_avg, int next_min, int margin)
{
    return margin > 0 && now > prev_avg + margin && next_min > prev_avg + margin / 2;
}

/*
 * A real break, then a show shorter than a show segment, then another
 * commercial shorter than a show segment, then a real show. The short
 * show is the rest of the break (the spots that did not score). The far
 * commercial is the show coming back, unless it is a strict ad length.
 */
static inline int break_tail_then_false_return(double break_len, double gap,
                                               double false_comm, double show_after,
                                               int false_strict, double min_show,
                                               double min_break)
{
    return break_len >= min_break
           && gap > 0.0 && gap < min_show
           && false_comm > 0.0 && false_comm < min_show
           && show_after > min_show
           && false_strict < 2;
}

/*
 * A commercial that opens on a held picture, then a show through to EOF
 * (or a short tail only). The hold is the credits; the show after it is
 * the next programme. A cut-heavy pod is the ads themselves and is not.
 */
static inline int credits_then_next_show(double comm_len, int opens_still,
                                        double show_after, double tail_len,
                                        double min_show, double min_comm)
{
    return opens_still && comm_len >= min_comm
           && show_after > min_show
           && tail_len >= 0.0 && tail_len < min_comm;
}

#endif
