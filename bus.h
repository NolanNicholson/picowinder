#ifndef BUS_H
#define BUS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
Scheduling of the two links to the Sidewinder base, kept free of hardware
access so it can be unit-tested on the host (see tests/).

The base's MCU both clocks out joystick packets (triggered by us) and receives
MIDI force commands. The original firmware did both concurrently: reads
free-ran at ~400 Hz and MIDI went out whenever the host sent something. That
overlap is the prime suspect for the base locking up 10-25 minutes into a
force-feedback session. adapt-ffb-joy, which doesn't show the problem, reads
every ~4.5 ms from a single loop, so its MIDI practically never overlaps a read.

Here reads are requested every BUS_READ_PERIOD_US, only once the UART is idle,
and queued MIDI bytes are released only in the gap between reads.
*/

#define BUS_READ_PERIOD_US   5000u  // ~200 Hz, like adapt-ffb-joy
#define BUS_READ_TIMEOUT_US  3000u  // a read normally completes well within this
#define BUS_MIDI_BYTE_US      320u  // 10 bits at 31250 baud
#define BUS_GUARD_US          300u  // quiet time before each read

struct bus
{
    uint32_t next_read_us;
    uint32_t uart_free_us;      // when the last byte we released finishes sending
    uint32_t read_started_us;
    uint32_t scan_at_trigger;
    bool reading;
};

void bus_init(struct bus *b, uint32_t now_us);

// Should a stick read be triggered now? uart_idle: the UART has finished
// shifting out everything (hardware BUSY flag clear).
bool bus_should_trigger(struct bus *b, uint32_t now_us, uint32_t scan_count, bool uart_idle);

// How many queued MIDI bytes may be written now without still being on the wire
// when the next read starts.
size_t bus_midi_budget(struct bus *b, uint32_t now_us, uint32_t scan_count, size_t pending);

// Record that n bytes were handed to the UART at now_us.
void bus_midi_sent(struct bus *b, uint32_t now_us, size_t n);

// ---- MIDI byte queue ------------------------------------------------------
// Messages are pushed whole or not at all, so a full queue never splits one.

#define MIDI_QUEUE_SIZE 4096

bool midi_queue_push(const uint8_t *data, size_t len);
size_t midi_queue_pop(uint8_t *out, size_t max);
size_t midi_queue_len(void);
void midi_queue_clear(void);

#endif // BUS_H
