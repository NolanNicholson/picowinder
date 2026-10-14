#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdbool.h>
#include <stdint.h>

/*
Base-reset watchdog decision logic, kept free of hardware access so it can be
unit-tested on the host (see tests/). main.c feeds it time and scan data and
carries out whatever action it returns.

The Sidewinder base's own MCU can reset or hang mid-session. After a reset it
turns its stock auto-center spring back on and forgets every effect we
downloaded; sometimes it also stops clocking out digital data.
*/

#define WATCHDOG_GAP_US          50000u   // inter-scan gap that means "base glitched"
#define WATCHDOG_REPLAY_MIN_US 2000000u   // never replay effects more often than this
#define WATCHDOG_KICK_US        100000u   // restart the read state machine after this stall
#define WATCHDOG_REINIT_US      500000u   // re-run the full handshake after this stall
#define WATCHDOG_REINIT_MIN_US 2000000u   // ...retried at most this often at first,
#define WATCHDOG_REINIT_MAX_US 30000000u  // backing off to this (doubling every 4 tries)

// Tracks the longest gap between consecutive completed scans. Updated from the
// scan IRQ using the IRQ's own timestamps, so a busy main loop (e.g. blocked on a
// long MIDI write) can never look like the base going quiet.
struct scan_tracker
{
    uint32_t count;
    uint32_t last_us;
    uint32_t pending_gap_us;
};

void scan_tracker_on_scan(volatile struct scan_tracker *t, uint32_t now_us);
// Returns and clears the longest gap seen since the last call. The caller must
// stop the scan IRQ from running concurrently (main.c disables interrupts).
uint32_t scan_tracker_take_gap(volatile struct scan_tracker *t);

enum watchdog_action
{
    WD_NONE,
    WD_REPLAY,  // base likely rebooted: stop all + rebuild effect memory
    WD_REINIT,  // base silent: full handshake, then rebuild effect memory
    WD_KICK,    // restart the read state machine
};

struct watchdog
{
    uint32_t last_scan_count;
    uint32_t last_scan_us;
    uint32_t last_kick_us;
    uint32_t last_reinit_us;
    uint32_t last_replay_us;
    bool replay_pending;     // a gap was seen; replay as soon as the rate limit allows
    bool ignore_next_gap;    // the gap spanning a reinit is ours, not a new glitch
    uint16_t reinit_streak;  // reinits since the base last answered
};

// Interval before the next reinit, given how many have failed in a row.
uint32_t watchdog_reinit_interval_us(uint16_t streak);

void watchdog_init(struct watchdog *w, uint32_t now_us, uint32_t scan_count);

// gap_us: longest inter-scan gap since the previous poll that saw new scans
// (from scan_tracker_take_gap), or 0.
enum watchdog_action watchdog_poll(struct watchdog *w, uint32_t now_us,
        uint32_t scan_count, uint32_t gap_us);

// Call after carrying out an action, with the time it finished.
void watchdog_done(struct watchdog *w, enum watchdog_action action, uint32_t now_us);

#endif // WATCHDOG_H
