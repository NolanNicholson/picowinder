#include "ffb_midi.h"


void ffb_midi_init(uart_inst_t *uart)
{
    const uint8_t start0[] = {
        0xc5, 0x01 // <ProgramChange> 0x01
    };

    const uint8_t start1[] = {
        0xf0, // SysEx begin
        0x00, 0x01, 0x0a, 0x01, 0x10, 0x05, 0x6b,
        0xf7  // SysEx end
    };

    const uint8_t start2[] = {
            0xb5, 0x40, 0x7f, // <ControlChange>(Modify, 0x7f)
            0xa5, 0x72, 0x57, // offset 0x72 := 0x57

            0xb5, 0x44, 0x7f,
            0xa5, 0x3c, 0x43,

            0xb5, 0x48, 0x7f,
            0xa5, 0x7e, 0x00,

            0xb5, 0x4c, 0x7f,
            0xa5, 0x04, 0x00,

            0xb5, 0x50, 0x7f,
            0xa5, 0x02, 0x00,

            0xb5, 0x54, 0x7f,
            0xa5, 0x02, 0x00,

            0xb5, 0x58, 0x7f,
            0xa5, 0x00, 0x7e,

            0xb5, 0x5c, 0x7f,
            0xa5, 0x3c, 0x00,

            0xb5, 0x60, 0x7f,
            0xa5, 0x14, 0x65,

            0xb5, 0x64, 0x7f,
            0xa5, 0x7e, 0x6b,

            0xb5, 0x68, 0x7f,
            0xa5, 0x36, 0x00,

            0xb5, 0x6c, 0x7f,
            0xa5, 0x28, 0x00,

            0xb5, 0x70, 0x7f,
            0xa5, 0x66, 0x4c,

            0xb5, 0x74, 0x7f,
            0xa5, 0x7e, 0x01
    };

    uart_write_blocking(uart, start0, sizeof(start0));
    sleep_ms(20);
    uart_write_blocking(uart, start1, sizeof(start1));
    sleep_ms(57);
    uart_write_blocking(uart, start2, sizeof(start2));
}

void ffb_midi_set_autocenter(uart_inst_t *uart, bool enabled)
{
    const uint8_t autocenter_on[] = {
            0xc5, 0x01
    };

    const uint8_t autocenter_off[] = {
            0xb5, 0x7c, 0x7f,
            0xa5, 0x7f, 0x00,
            0xc5, 0x06,
    };

    uart_write_blocking(uart, autocenter_on, sizeof(autocenter_on));

    if (!enabled)
    {
        sleep_ms(70);
        uart_write_blocking(uart, autocenter_off, sizeof(autocenter_off));
    }
}

/*
Since the MSB is reserved, we get 7 bits of real data per MIDI byte.
14-bit values must be split into high and low 7-bit chunks.
*/
static inline uint8_t lo7(uint16_t val) { return val & 0x7f; }
static inline uint8_t hi7(uint16_t val) { return (val >> 7) & 0x7f; }

void ffb_midi_define_effect(uart_inst_t *uart, struct Effect *effect)
{
    uint8_t effect_data[33] = {
        0xf0,                           // 0: SysEx start - effect data
        0x00, 0x01, 0x0a, 0x01, 0x23,   // 1..5: Effect header
        effect->type,                   // 6: enumerated effect type
        0x7f,                           // 7: not used?
        lo7(effect->duration),          // 8, 9: duration
        hi7(effect->duration),
        lo7(effect->button_mask),       // 10, 11: button mask
        hi7(effect->button_mask)
    };

    uint8_t next_index;

    switch (effect->type)
    {
        case CONSTANT:
        case SINE:
        case SQUARE:
        case RAMP:
        case TRIANGLE:
        case SAWTOOTHDOWN:
        case SAWTOOTHUP:

            effect_data[12] = lo7(effect->direction);   // 12, 13: direction
            effect_data[13] = hi7(effect->direction);
            effect_data[14] = effect->strength;         // 14: strength
            effect_data[15] = lo7(effect->sample_rate); // 15, 16: sample rate
            effect_data[16] = hi7(effect->sample_rate);
            effect_data[17] = 0x10;
            effect_data[18] = 0x4e;
            effect_data[19] = effect->attack;           // 19: Envelope Attack Start Level
            effect_data[20] = lo7(effect->attack_time); // 20, 21: Envelope Attack Time
            effect_data[21] = hi7(effect->attack_time);
            effect_data[22] = 0x7f;                     // 22: not used; must be 0x7f
            effect_data[23] = lo7(effect->fade_time);   // 23, 24: Envelope Fade Time
            effect_data[24] = hi7(effect->fade_time);
            effect_data[25] = effect->fade;             // 25: Envelope Fade End Level
            effect_data[26] = lo7(effect->frequency);   // 26, 27: Frequency
            effect_data[27] = hi7(effect->frequency);
            effect_data[28] = lo7(effect->amplitude);   // 28, 29: Amplitude
            effect_data[29] = hi7(effect->amplitude);
            effect_data[30] = 0x01;                     // 30, 31: ???
            effect_data[31] = 0x01;
            next_index = 32;

            break;

        case SPRING:
        case DAMPER:
        case INERTIA:

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

        case FRICTION:

            effect_data[12] = effect->strength_x;
            effect_data[13] = 0x00;
            effect_data[14] = effect->strength_y;
            effect_data[15] = 0x00;
            next_index = 16;

            break;
    }

    // Calculate and write checksum
    uint8_t checksum = 0;
    for (int i = 5; i < next_index; i++)
    {
        checksum += effect_data[i];
    }
    effect_data[next_index++] = 0x80 - (checksum & 0x7f);

    effect_data[next_index++] = 0xf7; // SysEx end

    uart_write_blocking(uart, effect_data, next_index);
}

void ffb_midi_play(uart_inst_t *uart, uint8_t effect_id)
{
    uint8_t msg[3] = { 0xb5, 0x20, effect_id };
    uart_write_blocking(uart, msg, sizeof(msg));
}

void ffb_midi_stop(uart_inst_t *uart, uint8_t effect_id)
{
    uint8_t msg[3] = { 0xb5, 0x30, effect_id };
    uart_write_blocking(uart, msg, sizeof(msg));
}

void ffb_midi_modify(uart_inst_t *uart, uint8_t effect_id, uint8_t param, uint16_t value)
{
    uint8_t msg[6] = {
        0xb5, param, effect_id, 0xa5, lo7(value), hi7(value) };
    uart_write_blocking(uart, msg, sizeof(msg));
}