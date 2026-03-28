#ifndef USB_DESCRIPTORS_H
#define USB_DESCRIPTORS_H

#include "common/tusb_common.h"
#include "device/usbd.h"

#include "config.h"

#define HID_UNIT_SECONDS 0x1003 // Documented as "Eng Lin:Time" in the PID spec


/////////////////////////////////////////////////////////////////////
// Input Report: Joystick postition, buttons, etc.
/////////////////////////////////////////////////////////////////////

#ifdef FIRMWARE_SHIFT
    #define NUM_BUTTONS 16

    // don't need any padding in this case
    #define BUTTON_PADDING HID_REPORT_COUNT(1)
#else
    #define NUM_BUTTONS 9

    #define BUTTON_PADDING \
        HID_REPORT_COUNT(1), \
        HID_REPORT_SIZE(7), \
        HID_INPUT(HID_CONSTANT | HID_ARRAY | HID_ABSOLUTE)
#endif

#define SIDEWINDER_REPORT_DESC_INPUT_JOYSTICK(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP), \
    HID_USAGE(HID_USAGE_DESKTOP_JOYSTICK), \
    HID_COLLECTION(HID_COLLECTION_APPLICATION), \
        /* Report ID */ __VA_ARGS__ \
        HID_USAGE_PAGE(HID_USAGE_PAGE_BUTTON), \
            HID_USAGE_MIN(1), \
            HID_USAGE_MAX(NUM_BUTTONS), \
            HID_LOGICAL_MIN(0), \
            HID_LOGICAL_MAX(1), \
            HID_REPORT_COUNT(NUM_BUTTONS), \
            HID_REPORT_SIZE(1), \
            HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
            \
            BUTTON_PADDING, \
            \
        HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP), \
            /* X and Y are both reported as unsigned 10-bit */ \
            HID_LOGICAL_MIN(0), \
            HID_LOGICAL_MAX_N(1023, 2), \
            /* X: 10 data bits, 6 padding bits */ \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(10), \
            HID_USAGE(HID_USAGE_DESKTOP_X), \
            HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(6), \
            HID_INPUT(HID_CONSTANT | HID_ARRAY | HID_ABSOLUTE), \
            /* Y: 10 data bits, 6 padding bits */ \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(10), \
            HID_USAGE(HID_USAGE_DESKTOP_Y), \
            HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(6), \
            HID_INPUT(HID_CONSTANT | HID_ARRAY | HID_ABSOLUTE), \
            /* Rz (joystick twist): 6 data bits, 2 padding bits */ \
            HID_LOGICAL_MIN(0), \
            HID_LOGICAL_MAX(63), \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(6), \
            HID_USAGE(HID_USAGE_DESKTOP_RZ), \
            HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(2), \
            HID_INPUT(HID_CONSTANT | HID_ARRAY | HID_ABSOLUTE), \
            /* Slider (throttle; kind of a dial): 7 data bits, 1 padding bit */ \
            HID_LOGICAL_MIN(0), \
            HID_LOGICAL_MAX(127), \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(7), \
            HID_USAGE(HID_USAGE_DESKTOP_SLIDER), \
            HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(1), \
            HID_INPUT(HID_CONSTANT | HID_ARRAY | HID_ABSOLUTE), \
            /* Hat switch: 4 data bits, 4 padding bits */ \
            HID_LOGICAL_MIN(1), \
            HID_LOGICAL_MAX(8), \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(4), \
            HID_USAGE(HID_USAGE_DESKTOP_HAT_SWITCH), \
            HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
            HID_REPORT_COUNT(1), \
            HID_REPORT_SIZE(4), \
            HID_INPUT(HID_CONSTANT | HID_ARRAY | HID_ABSOLUTE), \
            \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report: Set Effect - define params for an effect
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_SET_EFFECT(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_SET_EFFECT_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_INDEX), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(40), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(40), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Effect Type */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_TYPE), \
        HID_COLLECTION(HID_COLLECTION_LOGICAL), \
            HID_USAGE(HID_USAGE_PID_ET_CONSTANT_FORCE), \
            HID_USAGE(HID_USAGE_PID_ET_RAMP), \
            HID_USAGE(HID_USAGE_PID_ET_SQUARE), \
            HID_USAGE(HID_USAGE_PID_ET_SINE), \
            HID_USAGE(HID_USAGE_PID_ET_TRIANGLE), \
            HID_USAGE(HID_USAGE_PID_ET_SAWTOOTH_UP), \
            HID_USAGE(HID_USAGE_PID_ET_SAWTOOTH_DOWN), \
            HID_USAGE(HID_USAGE_PID_ET_SPRING), \
            HID_USAGE(HID_USAGE_PID_ET_DAMPER), \
            HID_USAGE(HID_USAGE_PID_ET_INERTIA), \
            HID_USAGE(HID_USAGE_PID_ET_FRICTION), \
            /* Unlike the FF2, ET_CUSTOM_FORCE is not supported here. */ \
            HID_LOGICAL_MIN(1), \
            HID_LOGICAL_MAX(11), \
            HID_PHYSICAL_MIN(1), \
            HID_PHYSICAL_MAX(11), \
            HID_REPORT_SIZE(8), \
            HID_REPORT_COUNT(1), \
            HID_OUTPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        \
        /* Duration, Trigger Repeat, Sample Period */ \
        HID_USAGE(HID_USAGE_PID_DURATION), \
        HID_USAGE(HID_USAGE_PID_TRIGGER_REPEAT_INTERVAL), \
        HID_USAGE(HID_USAGE_PID_SAMPLE_PERIOD), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX_N(0xffff, 3), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX_N(0xffff, 3), \
        HID_UNIT_N(HID_UNIT_SECONDS, 2), \
        HID_UNIT_EXPONENT(-3), \
        HID_REPORT_SIZE(16), \
        HID_REPORT_COUNT(3), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Gain */ \
        HID_USAGE(HID_USAGE_PID_GAIN), \
        HID_UNIT_EXPONENT(0), \
        HID_UNIT(0), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX(0x7f), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX_N(10000, 2), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Trigger Button */ \
        HID_USAGE(HID_USAGE_PID_TRIGGER_BUTTON), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(9), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(9), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Axes Enable */ \
        HID_USAGE(HID_USAGE_PID_AXES_ENABLE), \
        HID_COLLECTION(HID_COLLECTION_LOGICAL), \
            HID_USAGE_PAGE(HID_USAGE_PAGE_DESKTOP), \
            HID_USAGE(HID_USAGE_DESKTOP_X), \
            HID_USAGE(HID_USAGE_DESKTOP_Y), \
            HID_LOGICAL_MIN(0), \
            HID_LOGICAL_MAX(1), \
            HID_REPORT_SIZE(1), \
            HID_REPORT_COUNT(2), \
            HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        \
        /* Direction Enable */ \
        HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
        HID_USAGE(HID_USAGE_PID_DIRECTION_ENABLE), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        HID_REPORT_COUNT(5), \
        HID_OUTPUT(HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Direction */ \
        HID_USAGE(HID_USAGE_PID_DIRECTION), \
        HID_COLLECTION(HID_COLLECTION_LOGICAL), \
            /* NOTE: The HID spec specifies these should be Joystick usages, but Windows expects Ordinal.*/ \
            /* TODO: check Linux and try to reconcile */ \
            HID_USAGE_PAGE(HID_USAGE_PAGE_ORDINAL), \
            HID_USAGE(1), \
            HID_USAGE(2), \
            HID_UNIT(20), \
            HID_UNIT_EXPONENT(0), \
            HID_LOGICAL_MIN(0), \
            HID_LOGICAL_MAX_N(180, 2), \
            HID_PHYSICAL_MIN(0), \
            HID_PHYSICAL_MAX_N(360, 2), \
            HID_UNIT(0), \
            HID_REPORT_SIZE(8), \
            HID_REPORT_COUNT(2), \
            HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
            HID_UNIT_EXPONENT(0), \
            HID_UNIT(0), \
        HID_COLLECTION_END, \
        \
        /* Start Delay */ \
        HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
        HID_USAGE(HID_USAGE_PID_START_DELAY), \
        HID_UNIT_N(HID_UNIT_SECONDS, 2), \
        HID_UNIT_EXPONENT(-3), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX_N(32767, 2), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX_N(32767, 2), \
        HID_REPORT_SIZE(16), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        HID_UNIT(0), \
        HID_UNIT_EXPONENT(0), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report 2: Set Envelope - define envelope for an effect
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_SET_ENVELOPE(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_SET_ENVELOPE_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_INDEX), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(40), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(40), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Attack Level, Fade Level */ \
        HID_USAGE(HID_USAGE_PID_ATTACK_LEVEL), \
        HID_USAGE(HID_USAGE_PID_FADE_LEVEL), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX_N(255, 2), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX_N(10000, 2), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(2), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Attack Time, Fade Time */ \
        HID_USAGE(HID_USAGE_PID_ATTACK_TIME), \
        HID_USAGE(HID_USAGE_PID_FADE_TIME), \
        HID_UNIT_N(HID_UNIT_SECONDS, 2), \
        HID_UNIT_EXPONENT(-3), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX_N(32767, 2), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX_N(32767, 2), \
        HID_REPORT_SIZE(16), \
        HID_REPORT_COUNT(2), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        HID_PHYSICAL_MAX(0), \
        HID_UNIT(0), \
        HID_UNIT_EXPONENT(0), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report 3: Set Condition - params for spring/damper/inertia
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_SET_CONDITION(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_SET_CONDITION_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_INDEX), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(40), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(40), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Parameter Block Offset */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_OFFSET), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX(1), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX(1), \
        HID_REPORT_SIZE(8), /* 4 in SW FF2, since Specific Block Offset is also given afterward */ \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Center-Point Offset */ \
        HID_USAGE(HID_USAGE_PID_CENTER_POINT_OFFSET), \
        HID_LOGICAL_MIN(-128), \
        HID_LOGICAL_MAX(127), \
        HID_PHYSICAL_MIN_N(-10000, 2), \
        HID_PHYSICAL_MAX_N(10000, 2), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Positive/Negative Coefficient/Saturation; Dead Band */ \
        HID_USAGE(HID_USAGE_PID_POSITIVE_COEFFICIENT), \
        HID_USAGE(HID_USAGE_PID_NEGATIVE_COEFFICIENT), \
        HID_USAGE(HID_USAGE_PID_POSITIVE_SATURATION), \
        HID_USAGE(HID_USAGE_PID_NEGATIVE_SATURATION), \
        HID_USAGE(HID_USAGE_PID_DEAD_BAND), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX_N(255, 2), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX_N(10000, 2), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(5), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report 4: Set Periodic - set params for periodic wave effects
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_SET_PERIODIC(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_SET_PERIODIC_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_INDEX), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(40), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(40), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Magnitude */ \
        HID_USAGE(HID_USAGE_PID_MAGNITUDE), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX_N(255, 2), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX_N(10000, 2), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Offset */ \
        HID_USAGE(HID_USAGE_PID_OFFSET), \
        HID_LOGICAL_MIN(-128), \
        HID_LOGICAL_MAX(127), \
        HID_PHYSICAL_MIN_N(-10000, 2), \
        HID_PHYSICAL_MAX_N(10000, 2), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Phase */ \
        HID_USAGE(HID_USAGE_PID_PHASE), \
        HID_UNIT(20), \
        HID_UNIT_EXPONENT(-2), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX_N(255, 2), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX(0), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Period */ \
        HID_USAGE(HID_USAGE_PID_PERIOD), \
        HID_UNIT_N(HID_UNIT_SECONDS, 2), \
        HID_UNIT_EXPONENT(-3), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX_N(32767, 2), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX_N(32767, 2), \
        HID_REPORT_SIZE(16), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        HID_UNIT(0), \
        HID_UNIT_EXPONENT(0), \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report 5: Set Constant - describe a constant force
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_SET_CONSTANT(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_SET_CONSTANT_FORCE_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_INDEX), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(40), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(40), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Magnitude */ \
        HID_USAGE(HID_USAGE_PID_MAGNITUDE), \
        HID_LOGICAL_MIN_N(-255, 2), \
        HID_LOGICAL_MAX_N(255, 2), \
        HID_PHYSICAL_MIN_N(-10000, 2), \
        HID_PHYSICAL_MAX_N(10000, 2), \
        HID_REPORT_SIZE(16), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report 6: Set Ramp - describe a ramp force
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_SET_RAMP(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_SET_RAMP_FORCE_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_INDEX), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(40), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(40), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Ramp Start and End */ \
        HID_USAGE(HID_USAGE_PID_RAMP_START), \
        HID_USAGE(HID_USAGE_PID_RAMP_END), \
        HID_LOGICAL_MIN(-128), \
        HID_LOGICAL_MAX(127), \
        HID_PHYSICAL_MIN_N(-10000, 2), \
        HID_PHYSICAL_MAX_N(10000, 2), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(2), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report 10: Effect Operation - stop/start effects
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_EFFECT_OPERATION(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_EFFECT_OPERATION_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_INDEX), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(40), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(40), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Effect Operation */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_OPERATION), \
        HID_COLLECTION(HID_COLLECTION_LOGICAL), \
            HID_USAGE(HID_USAGE_PID_OP_EFFECT_START), \
            HID_USAGE(HID_USAGE_PID_OP_EFFECT_START_SOLO), \
            HID_USAGE(HID_USAGE_PID_OP_EFFECT_STOP), \
            HID_LOGICAL_MIN(1), \
            HID_LOGICAL_MAX(3), \
            HID_REPORT_SIZE(8), \
            HID_REPORT_COUNT(1), \
            HID_OUTPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        \
        /* Loop Count */ \
        HID_USAGE(HID_USAGE_PID_LOOP_COUNT), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX_N(255, 2), \
        HID_PHYSICAL_MIN(0), \
        HID_PHYSICAL_MAX_N(255, 2), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report 11: Block Free - delete an effect and free its space
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_BLOCK_FREE(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_FREE_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_INDEX), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(40), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(40), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report 12: Device Control - commands affecting entire device
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_DEVICE_CONTROL(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_DEVICE_CONTROL), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_DC_ENABLE_ACTUATORS), \
        HID_USAGE(HID_USAGE_PID_DC_DISABLE_ACTUATORS), \
        HID_USAGE(HID_USAGE_PID_DC_STOP_ALL_EFFECTS), \
        HID_USAGE(HID_USAGE_PID_DC_RESET), \
        HID_USAGE(HID_USAGE_PID_DC_PAUSE), \
        HID_USAGE(HID_USAGE_PID_DC_CONTINUE), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(6), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Output Report 13: Device Gain - gain across all effects
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_OUTPUT_DEVICE_GAIN(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_DEVICE_GAIN_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Device Gain */ \
        HID_USAGE(HID_USAGE_PID_DEVICE_GAIN), \
        HID_LOGICAL_MIN(0), \
        HID_LOGICAL_MAX(0x7f), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Feature Report 1: Create New Effect (in device memory)
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_FEATURE_CREATE_NEW_EFFECT(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_CREATE_NEW_EFFECT_PARAM_BLOCK_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Effect Type */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_TYPE), \
        HID_COLLECTION(HID_COLLECTION_LOGICAL), \
            HID_USAGE(HID_USAGE_PID_ET_CONSTANT_FORCE), \
            HID_USAGE(HID_USAGE_PID_ET_RAMP), \
            HID_USAGE(HID_USAGE_PID_ET_SQUARE), \
            HID_USAGE(HID_USAGE_PID_ET_SINE), \
            HID_USAGE(HID_USAGE_PID_ET_TRIANGLE), \
            HID_USAGE(HID_USAGE_PID_ET_SAWTOOTH_UP), \
            HID_USAGE(HID_USAGE_PID_ET_SAWTOOTH_DOWN), \
            HID_USAGE(HID_USAGE_PID_ET_SPRING), \
            HID_USAGE(HID_USAGE_PID_ET_DAMPER), \
            HID_USAGE(HID_USAGE_PID_ET_INERTIA), \
            HID_USAGE(HID_USAGE_PID_ET_FRICTION), \
            /* Unlike the FF2, ET_CUSTOM_FORCE is not supported here. */ \
            HID_LOGICAL_MIN(1), \
            HID_LOGICAL_MAX(11), \
            HID_PHYSICAL_MIN(1), \
            HID_PHYSICAL_MAX(11), \
            HID_REPORT_SIZE(8), \
            HID_REPORT_COUNT(1), \
            HID_FEATURE(HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        \
        /* Byte Count from FF2 descriptor omitted, since we don't support custom forces. */ \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Feature Report 2: Effect Block Load Report - was effect creation successful?
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_FEATURE_BLOCK_LOAD(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_LOAD_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* Block Index */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_INDEX), \
        HID_LOGICAL_MIN(1), \
        HID_LOGICAL_MAX(40), \
        HID_PHYSICAL_MIN(1), \
        HID_PHYSICAL_MAX(40), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Block Load Status */ \
        HID_USAGE(HID_USAGE_PID_EFFECT_PARAM_BLOCK_LOAD_STATUS), \
        HID_COLLECTION(HID_COLLECTION_LOGICAL), \
            HID_USAGE(HID_USAGE_PID_BLOCK_LOAD_SUCCESS), \
            HID_USAGE(HID_USAGE_PID_BLOCK_LOAD_FULL), \
            HID_USAGE(HID_USAGE_PID_BLOCK_LOAD_ERROR), \
            HID_LOGICAL_MIN(1), \
            HID_LOGICAL_MAX(3), \
            HID_PHYSICAL_MIN(1), \
            HID_PHYSICAL_MAX(3), \
            HID_REPORT_SIZE(8), \
            HID_REPORT_COUNT(1), \
            HID_FEATURE(HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        HID_COLLECTION_END, \
        \
        /* RAM Pool Available */ \
        HID_USAGE(HID_USAGE_PID_RAM_POOL_AVAILABLE), \
        HID_LOGICAL_MIN(0), \
        HID_PHYSICAL_MIN(0), \
        HID_LOGICAL_MAX_N(0xFFFF, 3), \
        HID_PHYSICAL_MAX_N(0xFFFF, 3), \
        HID_REPORT_SIZE(16), \
        HID_REPORT_COUNT(1), \
        HID_FEATURE(HID_DATA | HID_ARRAY | HID_ABSOLUTE), \
        \
    HID_COLLECTION_END


/////////////////////////////////////////////////////////////////////
// Feature Report 3: Pool size, max simultaneous effects, etc.
/////////////////////////////////////////////////////////////////////

#define SIDEWINDER_REPORT_DESC_FEATURE_POOL_REPORT(...) \
    HID_USAGE_PAGE(HID_USAGE_PAGE_PID), \
    HID_USAGE(HID_USAGE_PID_PARAM_BLOCK_POOLS_REPORT), \
    HID_COLLECTION(HID_COLLECTION_LOGICAL), \
        /* Report ID */ __VA_ARGS__ \
        \
        /* RAM Pool Size */ \
        HID_USAGE(HID_USAGE_PID_RAM_POOL_SIZE), \
        HID_REPORT_SIZE(16), \
        HID_REPORT_COUNT(1), \
        HID_LOGICAL_MIN(0), \
        HID_PHYSICAL_MIN(0), \
        HID_LOGICAL_MAX_N(0xFFFF, 3), \
        HID_PHYSICAL_MAX_N(0xFFFF, 3), \
        HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Simultaneous Maximum Effects */ \
        HID_USAGE(HID_USAGE_PID_SIMULTANEOUS_EFFECTS_MAX), \
        HID_LOGICAL_MAX_N(255, 2), \
        HID_PHYSICAL_MAX_N(255, 2), \
        HID_REPORT_SIZE(8), \
        HID_REPORT_COUNT(1), \
        HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* Managed Pool, Shared Blocks */ \
        HID_USAGE(HID_USAGE_PID_DEVICE_MANAGED_POOL), \
        HID_USAGE(HID_USAGE_PID_SHARED_PARAMETER_BLOCKS), \
        HID_LOGICAL_MAX(1), \
        HID_PHYSICAL_MAX(1), \
        HID_REPORT_SIZE(1), \
        HID_REPORT_COUNT(2), \
        HID_FEATURE(HID_DATA | HID_VARIABLE | HID_ABSOLUTE), \
        \
        /* padding */ \
        HID_REPORT_SIZE(6), \
        HID_REPORT_COUNT(1), \
        HID_FEATURE(HID_CONSTANT | HID_VARIABLE | HID_ABSOLUTE), \
        \
    HID_COLLECTION_END

#endif // USB_DESCRIPTORS_H