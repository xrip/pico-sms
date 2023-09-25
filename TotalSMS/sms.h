#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sms_types.h"

// for frontends to detect if the game (maybe) uses sram:
// 1: on rom load, check if a .sav exits, if so, load it
// 2: on exit, call SMS_used_sram(), if true, write [game].sav

// todo: support bios loading
// add flag [bool has_bios]
// on bios load, reset all regs to 0x00
// on rom load, check if has bios, before resetting regs
// on rom map change, check if has bios AND if bios is mapped

bool SMS_init();
bool SMS_loadbios( const uint8_t* bios, size_t size);
bool SMS_loadrom( const uint8_t* rom, size_t size, int system_hint);
void SMS_run( size_t cycles);

bool SMS_loadsave(const uint8_t* data, size_t size);
bool SMS_used_sram();

void SMS_skip_frame( bool enable);
void SMS_set_pixels( void* pixels, uint16_t pitch, uint8_t bpp);
void SMS_set_apu_callback( sms_apu_callback_t cb, uint32_t freq);
void SMS_set_vblank_callback( sms_vblank_callback_t cb);
void SMS_set_colour_callback( sms_colour_callback_t cb);
void SMS_set_userdata( void* userdata);

bool SMS_savestate(struct SMS_State* state);
bool SMS_loadstate(const struct SMS_State* state);

// i had a bug in the noise channel which made the drums in all games sound *much*
// better, so much so, that i assumed other emulators emulated the noise channel wrong!
// however, after listening to real hw, the drums were in fact always that bad sounding.
// setting this to true will re-enable better drums!
void SMS_set_better_drums( bool enable);

void SMS_set_system_type( enum SMS_System system);
enum SMS_System SMS_get_system_type();
bool SMS_is_system_type_sms();
bool SMS_is_system_type_gg();
bool SMS_is_system_type_sg();

void SMS_get_pixel_region(int* x, int* y, int* w, int* h);

// [INPUT]
void SMS_set_port_a( enum SMS_PortA pin, bool down);
void SMS_set_port_b( enum SMS_PortB pin, bool down);

uint32_t SMS_crc32(uint32_t crc, const void* data, size_t size);

void z80_run();
void vdp_run(const uint8_t cycles);
void psg_run(const uint8_t cycles);
void psg_sync();

#ifdef __cplusplus
}
#endif
