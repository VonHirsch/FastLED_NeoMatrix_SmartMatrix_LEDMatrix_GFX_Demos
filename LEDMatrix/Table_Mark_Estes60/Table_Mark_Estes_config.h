// --------------------------- Config Start --------------------------------
// Randomly slowdown animation. Nice in principle, but not when debugging performance
#define RANDOMSLOWME

// Control whether pattern order is random from the start or after one pass
#define MIXIT_AFTER_FIRST_PASS

// write the pattern number in upper left (makes more sense on higher res displays)
#define SHOW_PATTERN_NUM
//
// Display FPS computed by Framebuffer::GFX
//#define DISPLAY_FPS

// Some pattern transitions look weird without a clear in between
//#define CLEAR_BETWEEN_PATTERNS

// This allows a selection of only my favourite patterns.
// Comment this out to get all the patterns -- merlin
//#define BESTPATTERNS
#ifdef BESTPATTERNS

uint8_t bestpatterns[] = { 3, 4, 5, 9, 10, 11, 12, 16, 17, 19, 21, 23,25,30,  53,54,  55, 58, 59, 61,67,  69, 70, 72,73,77, 80 81, 82,85, 86, 87,88, 90,94,96,98,101, 102,103, 108, 109, 111, 115, 124, 134, 139,143, 155};
#define numbest           sizeof(bestpatterns)
#define lastpatindex numbest
#else
#define lastpatindex mpatterns
#endif

// By default audio support is on for what's expected to be teensy.
#define TME_AUDIO
// But turn it off on ARDUINOONPC / Raspberry Pi and ESP32
#ifdef ARDUINOONPC
#undef ARDUINOONPC
#endif


#ifdef ESP32
#undef TME_AUDIO
#endif

// Don't load non existent EasyTransfer for platforms without audio
#ifdef TME_AUDIO
#include <EasyTransfer.h>// used for exchange with 2nd arduino for audio processing
#endif

#define TIMING              90
//seconds per pattern
#define LATCH               23

int16_t pattern = 141;//this picks the pattern to start with...
// --------------------------- Config End ----------------------------------
