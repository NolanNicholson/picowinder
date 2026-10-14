#include "bus.h"

void bus_init(struct bus *b, uint32_t now_us)
{
    b->next_read_us = now_us;
    b->uart_free_us = now_us;
    b->read_started_us = now_us;
    b->scan_at_trigger = 0;
    b->reading = false;
}

static inline bool reached(uint32_t now_us, uint32_t t) { return (int32_t)(now_us - t) >= 0; }

static void update_read_state(struct bus *b, uint32_t now_us, uint32_t scan_count)
{
    if (b->reading
            && (scan_count != b->scan_at_trigger
                || (now_us - b->read_started_us) > BUS_READ_TIMEOUT_US))
    {
        // Packet arrived, or the base didn't answer (the watchdog deals with that).
        b->reading = false;
    }
}

bool bus_should_trigger(struct bus *b, uint32_t now_us, uint32_t scan_count, bool uart_idle)
{
    update_read_state(b, now_us, scan_count);

    if (b->reading || !reached(now_us, b->next_read_us)) { return false; }
    if (!uart_idle || !reached(now_us, b->uart_free_us)) { return false; }

    b->reading = true;
    b->read_started_us = now_us;
    b->scan_at_trigger = scan_count;
    b->next_read_us = now_us + BUS_READ_PERIOD_US;
    return true;
}

size_t bus_midi_budget(struct bus *b, uint32_t now_us, uint32_t scan_count, size_t pending)
{
    update_read_state(b, now_us, scan_count);
    if (b->reading || pending == 0) { return 0; }

    uint32_t start = reached(now_us, b->uart_free_us) ? now_us : b->uart_free_us;
    uint32_t deadline = b->next_read_us - BUS_GUARD_US;
    if (!reached(deadline, start)) { return 0; }

    size_t n = (deadline - start) / BUS_MIDI_BYTE_US;
    return n < pending ? n : pending;
}

void bus_midi_sent(struct bus *b, uint32_t now_us, size_t n)
{
    uint32_t start = reached(now_us, b->uart_free_us) ? now_us : b->uart_free_us;
    b->uart_free_us = start + (uint32_t)n * BUS_MIDI_BYTE_US;
}

// ---- MIDI byte queue ------------------------------------------------------

static uint8_t queue[MIDI_QUEUE_SIZE];
static size_t q_head, q_len;   // head = index of oldest byte

bool midi_queue_push(const uint8_t *data, size_t len)
{
    if (len > MIDI_QUEUE_SIZE - q_len) { return false; }
    for (size_t i = 0; i < len; i++)
    {
        queue[(q_head + q_len + i) % MIDI_QUEUE_SIZE] = data[i];
    }
    q_len += len;
    return true;
}

size_t midi_queue_pop(uint8_t *out, size_t max)
{
    size_t n = max < q_len ? max : q_len;
    for (size_t i = 0; i < n; i++)
    {
        out[i] = queue[(q_head + i) % MIDI_QUEUE_SIZE];
    }
    q_head = (q_head + n) % MIDI_QUEUE_SIZE;
    q_len -= n;
    return n;
}

size_t midi_queue_len(void) { return q_len; }

void midi_queue_clear(void) { q_head = 0; q_len = 0; }
