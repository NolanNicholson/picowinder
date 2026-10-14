#include <string.h>

#include "ffb_midi.h"

enum MidiEffectType effects_assigned[EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE] = { 0 };

// Last value sent for each (effect, modify parameter), or -1 if none. Games
// resend unchanged parameters many times a second, and MIDI runs at only
// 31250 baud, so modifies that wouldn't change anything are skipped. A row is
// cleared whenever its slot is (re)allocated or freed.
static int32_t modify_cache[EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE][0x80];

static inline bool is_real_effect_slot(int effect_id)
{
    return effect_id >= EFFECT_MEMORY_START
        && effect_id < EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE;
}

static void clear_modify_cache(int effect_id)
{
    for (int p = 0; p < 0x80; p++) { modify_cache[effect_id][p] = -1; }
}

// Shadow copy of everything sent to the base, so its effect memory can be
// rebuilt after it resets, without the host noticing. See ffb_midi_replay_state().
// Longest effect SysEx: 32 bytes of header/params + checksum + 0xf7.
#define MAX_EFFECT_SYSEX 34
static uint8_t effect_sysex[EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE][MAX_EFFECT_SYSEX];
static uint8_t effect_sysex_len[EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE];
static bool effect_playing[EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE];
static uint32_t effect_play_start_ms[EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE];

// Last value sent for each MIDI_ALL_EFFECTS modify (e.g. device gain), for replay.
static int32_t global_modify_cache[0x80];
static bool global_modify_cache_init = false;

static void set_playing(int effect_id, bool playing)
{
    if (!is_real_effect_slot(effect_id)) { return; }
    effect_playing[effect_id] = playing;
    if (playing) { effect_play_start_ms[effect_id] = to_ms_since_boot(get_absolute_time()); }
}

int ffb_midi_get_free_effect_id()
{
    for (int i = EFFECT_MEMORY_START; i < EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE; i++)
    {
        if (effects_assigned[i] == MIDI_ET_NONE) { return i; }
    }

    return -1;
}

size_t ffb_midi_get_num_available_effects()
{
    size_t num_free = 0;

    for (int i = EFFECT_MEMORY_START; i < EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE; i++)
    {
        if (effects_assigned[i] == MIDI_ET_NONE) { num_free++; }
    }

    return num_free;
}

bool last_add_succeeded;

bool ffb_midi_last_add_succeeded()
{
    return last_add_succeeded;
}

uint8_t last_assigned_effect_id;

uint8_t ffb_midi_last_assigned_effect_id()
{
    return last_assigned_effect_id;
}

void ffb_midi_set_autocenter(uart_inst_t *uart, bool enabled)
{
    uint8_t autocenter_cmd[] = {
            0xc5, 0x01
    };

    autocenter_cmd[1] = enabled ? 0x01 : 0x06; 
    midi_write(uart, autocenter_cmd, sizeof(autocenter_cmd));
}

/*
Since the MSB is reserved, we get 7 bits of real data per MIDI byte.
14-bit values must be split into high and low 7-bit chunks.
*/
static inline uint8_t lo7(uint16_t val) { return val & 0x7f; }
static inline uint8_t hi7(uint16_t val) { return (val >> 7) & 0x7f; }

int ffb_midi_define_effect(uart_inst_t *uart, struct Effect *effect)
{
    // Get an effect id (and make sure we have enough room)
    int effect_id = ffb_midi_get_free_effect_id();
    if (effect_id < 0)
    {
        last_add_succeeded = false;
        return effect_id;
    }

    // At a glance, 0x24 through 0x2f will start the effect immediately,
    // 0x20 through 0x23 will wait for it to be started,
    // and any upper nibble besides 0x2* will fail.
    uint8_t flags = effect->play_immediately ? 0x24 : 0x23;

    uint8_t effect_data[MAX_EFFECT_SYSEX] = {
        0xf0,                           // 0: SysEx start - effect data
        0x00, 0x01, 0x0a, 0x01,         // 1..4: Effect header
        flags,                          // 5: Effect flags?
        effect->type,                   // 6: enumerated effect type
        0x7f,                           // 7: Overwrite existing effect ID (0x7f to claim new effect)
        lo7(effect->duration),          // 8, 9: duration
        hi7(effect->duration),
        lo7(effect->button_mask),       // 10, 11: button mask
        hi7(effect->button_mask)
    };

    uint8_t next_index;

    switch (effect->type)
    {
        case MIDI_ET_CONSTANT:
        case MIDI_ET_SINE:
        case MIDI_ET_SQUARE:
        case MIDI_ET_RAMP:
        case MIDI_ET_TRIANGLE:
        case MIDI_ET_SAWTOOTHDOWN:
        case MIDI_ET_SAWTOOTHUP:

            effect_data[12] = lo7(effect->direction);   // 12, 13: direction
            effect_data[13] = hi7(effect->direction);
            effect_data[14] = effect->gain;             // 14: gain
            effect_data[15] = lo7(effect->sample_rate); // 15, 16: sample rate
            effect_data[16] = hi7(effect->sample_rate);
            effect_data[17] = 0x10;                     // 17, 18: truncate; 0x4e10 = 10000 = full waveform
            effect_data[18] = 0x4e;
            effect_data[19] = effect->attack_level;     // 19: Envelope Attack Start Level
            effect_data[20] = lo7(effect->attack_time); // 20, 21: Envelope Attack Time
            effect_data[21] = hi7(effect->attack_time);
            effect_data[22] = effect->sustain_level;    // 22: Envelope Sustain Level
            effect_data[23] = lo7(effect->fade_time);   // 23, 24: Envelope Fade Time
            effect_data[24] = hi7(effect->fade_time);
            effect_data[25] = effect->fade_level;       // 25: Envelope Fade End Level
            effect_data[26] = lo7(effect->frequency);   // 26, 27: Frequency
            effect_data[27] = hi7(effect->frequency);
            effect_data[28] = lo7(effect->amplitude);   // 28, 29: Amplitude
            effect_data[29] = hi7(effect->amplitude);
            effect_data[30] = 0x01;                     // 30, 31: ???
            effect_data[31] = 0x01;
            next_index = 32;

            break;

        case MIDI_ET_SPRING:
        case MIDI_ET_DAMPER:
        case MIDI_ET_INERTIA:

            effect_data[12] = effect->strength_x;
            effect_data[13] = 0x00;
            effect_data[14] = effect->strength_y;
            effect_data[15] = 0x00;
            effect_data[16] = lo7(effect->offset_x);
            effect_data[17] = hi7(effect->offset_x);
            effect_data[18] = lo7(effect->offset_y);
            effect_data[19] = hi7(effect->offset_y);
            next_index = 20;

            break;

        case MIDI_ET_FRICTION:

            effect_data[12] = effect->strength_x;
            effect_data[13] = 0x00;
            effect_data[14] = effect->strength_y;
            effect_data[15] = 0x00;
            next_index = 16;

            break;

        default:
            // Unknown type: send nothing and report failure.
            last_add_succeeded = false;
            return -1;
    }

    // Calculate and write checksum
    uint8_t checksum = 0;
    for (int i = 5; i < next_index; i++)
    {
        checksum += effect_data[i];
    }
    effect_data[next_index++] = 0x80 - (checksum & 0x7f);

    effect_data[next_index++] = 0xf7; // SysEx end

    midi_write(uart, effect_data, next_index);

    memcpy(effect_sysex[effect_id], effect_data, next_index);
    effect_sysex_len[effect_id] = next_index;
    set_playing(effect_id, effect->play_immediately);

    last_add_succeeded = true;
    last_assigned_effect_id = effect_id;
    effects_assigned[effect_id] = effect->type;
    // Forget cached values from the slot's previous effect.
    clear_modify_cache(effect_id);
    return effect_id;
}

void ffb_midi_erase(uart_inst_t *uart, int effect_id)
{
    if (effect_id < 0) { return; }

    uint8_t msg[3] = { 0xb5, 0x10, effect_id & 0x7f };
    midi_write(uart, msg, sizeof(msg));

    if (is_real_effect_slot(effect_id))
    {
        effects_assigned[effect_id] = MIDI_ET_NONE;
        clear_modify_cache(effect_id);
        effect_playing[effect_id] = false;
    }
}

void ffb_midi_play_solo(uart_inst_t *uart, int effect_id)
{
    if (effect_id < 0) { return; }

    uint8_t msg[3] = { 0xb5, 0x00, effect_id & 0x7f };
    midi_write(uart, msg, sizeof(msg));

    for (int i = EFFECT_MEMORY_START; i < EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE; i++)
    {
        effect_playing[i] = false;
    }
    set_playing(effect_id, true);
}

void ffb_midi_play(uart_inst_t *uart, int effect_id)
{
    if (effect_id < 0) { return; }

    uint8_t msg[3] = { 0xb5, 0x20, effect_id & 0x7f };
    midi_write(uart, msg, sizeof(msg));

    set_playing(effect_id, true);
}

void ffb_midi_pause(uart_inst_t *uart, int effect_id)
{
    if (effect_id < 0) { return; }

    uint8_t msg[3] = { 0xb5, 0x30, effect_id & 0x7f };
    midi_write(uart, msg, sizeof(msg));

    set_playing(effect_id, false);
}

void ffb_midi_modify(uart_inst_t *uart, int effect_id, uint8_t param, uint16_t value)
{
    if (effect_id < 0) { return; }

    // Only real effect slots are deduplicated; device-wide modifies (0x7f)
    // always go out.
    if (is_real_effect_slot(effect_id) && param < 0x80)
    {
        if (modify_cache[effect_id][param] == (int32_t)value) { return; }
        modify_cache[effect_id][param] = (int32_t)value;
    }
    else if (effect_id == MIDI_ALL_EFFECTS && param < 0x80)
    {
        if (!global_modify_cache_init)
        {
            for (int p = 0; p < 0x80; p++) { global_modify_cache[p] = -1; }
            global_modify_cache_init = true;
        }
        global_modify_cache[param] = (int32_t)value;
    }

    uint8_t msg[6] = {
        0xb5, param, effect_id & 0x7f, 0xa5, lo7(value), hi7(value) };
    midi_write(uart, msg, sizeof(msg));
}

static void send_modify_raw(uart_inst_t *uart, int effect_id, uint8_t param, uint16_t value)
{
    uint8_t msg[6] = {
        0xb5, param, effect_id & 0x7f, 0xa5, lo7(value), hi7(value) };
    midi_write(uart, msg, sizeof(msg));
}

// Is this effect still running? Finite effects stop by themselves on the
// base, so one whose duration has elapsed is not restarted.
static bool effect_still_playing(int effect_id)
{
    if (!effect_playing[effect_id]) { return false; }

    int32_t duration = modify_cache[effect_id][MODIFY_DURATION];
    if (duration < 0)
    {
        duration = effect_sysex[effect_id][8] | (effect_sysex[effect_id][9] << 7);
    }
    if (duration == 0) { return true; } // infinite

    uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - effect_play_start_ms[effect_id];
    return elapsed < (uint32_t)duration * 2;
}

void ffb_midi_replay_state(uart_inst_t *uart)
{
    // The base hands out effect IDs itself (lowest free slot), and the host
    // keeps using the IDs it already has. Redefine every slot in ascending
    // order; holes get a placeholder that is erased afterwards, so every effect
    // lands on its old ID.
    int last_used = -1;
    for (int i = EFFECT_MEMORY_START; i < EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE; i++)
    {
        if (effects_assigned[i] != MIDI_ET_NONE) { last_used = i; }
    }

    // Erase every slot, not just ours, in case the base kept effects we no
    // longer know about.
    for (int i = EFFECT_MEMORY_START; i < EFFECT_MEMORY_START + EFFECT_MEMORY_SIZE; i++)
    {
        uint8_t msg[3] = { 0xb5, 0x10, i & 0x7f };
        midi_write(uart, msg, sizeof(msg));
    }

    for (int i = EFFECT_MEMORY_START; i <= last_used; i++)
    {
        if (effects_assigned[i] != MIDI_ET_NONE)
        {
            // Redefine without auto-play; we restore play state below.
            uint8_t data[MAX_EFFECT_SYSEX];
            uint8_t len = effect_sysex_len[i];
            memcpy(data, effect_sysex[i], len);
            data[5] = 0x23;
            uint8_t checksum = 0;
            for (int j = 5; j < len - 2; j++) { checksum += data[j]; }
            data[len - 2] = 0x80 - (checksum & 0x7f);
            midi_write(uart, data, len);
        }
        else
        {
            // Placeholder: zero-strength friction, not playing.
            uint8_t data[18] = {
                0xf0, 0x00, 0x01, 0x0a, 0x01,
                0x23, MIDI_ET_FRICTION, 0x7f,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0xf7 };
            uint8_t checksum = 0;
            for (int j = 5; j < 16; j++) { checksum += data[j]; }
            data[16] = 0x80 - (checksum & 0x7f);
            midi_write(uart, data, sizeof(data));
        }
    }

    for (int i = EFFECT_MEMORY_START; i <= last_used; i++)
    {
        if (effects_assigned[i] == MIDI_ET_NONE)
        {
            uint8_t msg[3] = { 0xb5, 0x10, i & 0x7f };
            midi_write(uart, msg, sizeof(msg));
            continue;
        }

        for (int p = 0; p < 0x80; p++)
        {
            if (modify_cache[i][p] >= 0) { send_modify_raw(uart, i, p, modify_cache[i][p]); }
        }

        if (effect_still_playing(i)) { ffb_midi_play(uart, i); }
        else { effect_playing[i] = false; }
    }

    if (global_modify_cache_init)
    {
        for (int p = 0; p < 0x80; p++)
        {
            if (global_modify_cache[p] >= 0) { send_modify_raw(uart, MIDI_ALL_EFFECTS, p, global_modify_cache[p]); }
        }
    }
}
