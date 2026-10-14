#include "watchdog.h"

void scan_tracker_on_scan(volatile struct scan_tracker *t, uint32_t now_us)
{
    if (t->count != 0)
    {
        uint32_t gap = now_us - t->last_us;
        if (gap > t->pending_gap_us) { t->pending_gap_us = gap; }
    }
    t->last_us = now_us;
    t->count++;
}

uint32_t scan_tracker_take_gap(volatile struct scan_tracker *t)
{
    uint32_t gap = t->pending_gap_us;
    t->pending_gap_us = 0;
    return gap;
}

void watchdog_init(struct watchdog *w, uint32_t now_us, uint32_t scan_count)
{
    w->last_scan_count = scan_count;
    w->last_scan_us = now_us;
    w->last_kick_us = now_us;
    w->last_reinit_us = now_us;
    w->last_replay_us = now_us - WATCHDOG_REPLAY_MIN_US;
    w->replay_pending = false;
    w->ignore_next_gap = false;
    w->reinit_streak = 0;
}

uint32_t watchdog_reinit_interval_us(uint16_t streak)
{
    // A hung base may need quiet time to recover, so back off.
    uint32_t interval = WATCHDOG_REINIT_MIN_US << (streak / 4 > 4 ? 4 : streak / 4);
    return interval > WATCHDOG_REINIT_MAX_US ? WATCHDOG_REINIT_MAX_US : interval;
}

enum watchdog_action watchdog_poll(struct watchdog *w, uint32_t now_us,
        uint32_t scan_count, uint32_t gap_us)
{
    if (scan_count != w->last_scan_count)
    {
        w->last_scan_count = scan_count;
        w->last_scan_us = now_us;
        w->last_kick_us = now_us;
        if (gap_us >= WATCHDOG_GAP_US && !w->ignore_next_gap) { w->replay_pending = true; }
        w->ignore_next_gap = false;
        w->reinit_streak = 0;
    }
    else if ((now_us - w->last_scan_us) > WATCHDOG_REINIT_US
            && (now_us - w->last_reinit_us) > watchdog_reinit_interval_us(w->reinit_streak))
    {
        return WD_REINIT;
    }
    else if ((now_us - w->last_kick_us) > WATCHDOG_KICK_US)
    {
        return WD_KICK;
    }

    // Rate-limited: a replay costs ~100 ms of MIDI and must never be able to
    // feed back into itself. A gap seen during the cooldown is remembered, not
    // dropped, so a second reset shortly after the first still gets repaired.
    if (w->replay_pending && (now_us - w->last_replay_us) > WATCHDOG_REPLAY_MIN_US)
    {
        return WD_REPLAY;
    }

    return WD_NONE;
}

void watchdog_done(struct watchdog *w, enum watchdog_action action, uint32_t now_us)
{
    switch (action)
    {
        case WD_REPLAY:
            w->last_replay_us = now_us;
            w->replay_pending = false;
            break;
        case WD_REINIT:
            // Effects were already replayed; don't redo it when data resumes.
            w->last_reinit_us = now_us;
            w->reinit_streak++;
            w->last_replay_us = now_us;
            w->replay_pending = false;
            w->ignore_next_gap = true;
            w->last_scan_us = now_us;
            w->last_kick_us = now_us;
            break;
        case WD_KICK:
            w->last_kick_us = now_us;
            break;
        case WD_NONE:
            break;
    }
}
