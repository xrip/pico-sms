#include <cstdio>
#include "pico/stdlib.h"
#include "pico/runtime.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "hardware/vreg.h"
#include "hardware/watchdog.h"
#include <hardware/sync.h>
#include <hardware/pwm.h>
#include <pico/multicore.h>
#include <hardware/flash.h>
#include <cstring>
#include <cstdarg>

#include "pico/multicore.h"
#include "sms.h"
#include "vga.h"

#define ENABLE_SOUND 1
#if ENABLE_SOUND

#include "audio.h"

#endif

#include "nespad.h"
#include "f_util.h"
#include "ff.h"
#include "VGA_ROM_F16.h"
#include "ps2kbd_mrmltr.h"

#ifndef OVERCLOCKING
#define OVERCLOCKING 270
#endif

#pragma GCC optimize("Ofast")

#define FLASH_TARGET_OFFSET (1024 * 1024)
const char *rom_filename = (const char *) (XIP_BASE + FLASH_TARGET_OFFSET);
const uint8_t *rom = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET) + 4096;

static size_t rom_size = 0;
static FATFS fs;

#define SHOW_FPS 1

bool is_gg = false;

static struct SMS_Core sms = { 0 };
static const sVmode *vmode = nullptr;
struct semaphore vga_start_semaphore;

uint8_t SCREEN[SMS_SCREEN_WIDTH * SMS_SCREEN_HEIGHT];
char textmode[30][80];
uint8_t colors[30][80];

struct input_bits_t {
    bool a: true;
    bool b: true;
    bool select: true;
    bool start: true;
    bool right: true;
    bool left: true;
    bool up: true;
    bool down: true;
};
static input_bits_t keyboard_bits = {false, false, false, false, false, false, false, false};
static input_bits_t gamepad1_bits = {false, false, false, false, false, false, false, false};
static input_bits_t gamepad2_bits = {false, false, false, false, false, false, false, false};

void nespad_tick() {
    nespad_read();

    gamepad1_bits.a = (nespad_state & DPAD_A) != 0;
    gamepad1_bits.b = (nespad_state & DPAD_B) != 0;
    gamepad1_bits.select = (nespad_state & DPAD_SELECT) != 0;
    gamepad1_bits.start = (nespad_state & DPAD_START) != 0;
    gamepad1_bits.up = (nespad_state & DPAD_UP) != 0;
    gamepad1_bits.down = (nespad_state & DPAD_DOWN) != 0;
    gamepad1_bits.left = (nespad_state & DPAD_LEFT) != 0;
    gamepad1_bits.right = (nespad_state & DPAD_RIGHT) != 0;

}

static bool isInReport(hid_keyboard_report_t const *report, const unsigned char keycode) {
    for (unsigned char i: report->keycode) {
        if (i == keycode) {
            return true;
        }
    }
    return false;
}

void
__not_in_flash_func(process_kbd_report)(hid_keyboard_report_t const *report, hid_keyboard_report_t const *prev_report) {
    /* printf("HID key report modifiers %2.2X report ", report->modifier);
    for (unsigned char i: report->keycode)
        printf("%2.2X", i);
    printf("\r\n");
     */
    keyboard_bits.start = isInReport(report, HID_KEY_ENTER);
    keyboard_bits.select = isInReport(report, HID_KEY_BACKSPACE);
    keyboard_bits.a = isInReport(report, HID_KEY_Z);
    keyboard_bits.b = isInReport(report, HID_KEY_X);
    keyboard_bits.up = isInReport(report, HID_KEY_ARROW_UP);
    keyboard_bits.down = isInReport(report, HID_KEY_ARROW_DOWN);
    keyboard_bits.left = isInReport(report, HID_KEY_ARROW_LEFT);
    keyboard_bits.right = isInReport(report, HID_KEY_ARROW_RIGHT);
    //-------------------------------------------------------------------------
}

Ps2Kbd_Mrmltr ps2kbd(
        pio1,
        0,
        process_kbd_report);

typedef enum {
    RESOLUTION_NATIVE,
    RESOLUTION_TEXTMODE,
} resolution_t;
resolution_t resolution = RESOLUTION_TEXTMODE;

void draw_text(char *text, uint8_t x, uint8_t y, uint8_t color, uint8_t bgcolor) {
    uint8_t len = strlen(text);
    len = len < 80 ? len : 80;
    memcpy(&textmode[y][x], text, len);
    memset(&colors[y][x], (color << 4) | (bgcolor & 0xF), len);
}

/**
 * Load a .gb rom file in flash from the SD card
 */
void load_cart_rom_file(char *filename) {
#if 0
    if (strcmp(rom_filename, filename) == 0) {
        printf("Launching last rom");
        return;
    }
#endif
    FIL fil;
    FRESULT fr;

    size_t bufsize = sizeof(SCREEN) & 0xfffff000;
    BYTE *buffer = (BYTE *) SCREEN;
    auto ofs = FLASH_TARGET_OFFSET;

    FILINFO fi;
    f_stat(filename, &fi);
    rom_size = fi.fsize;

    printf("Writing %s rom to flash %x\r\n", filename, ofs);
    fr = f_open(&fil, filename, FA_READ);

    UINT bytesRead;
    if (fr == FR_OK) {
        uint32_t ints = save_and_disable_interrupts();
        multicore_lockout_start_blocking();

        // TODO: Save it after success loading to prevent corruptions
        printf("Flashing %d bytes to flash address %x\r\n", 256, ofs);
        flash_range_erase(ofs, 4096);
        flash_range_program(ofs, reinterpret_cast<const uint8_t *>(filename), 256);


        ofs += 4096;
        for (;;) {
            gpio_put(PICO_DEFAULT_LED_PIN, true);
            fr = f_read(&fil, buffer, bufsize, &bytesRead);
            if (fr == FR_OK) {
                if (bytesRead == 0) {
                    break;
                }

                printf("Flashing %d bytes to flash address %x\r\n", bytesRead, ofs);

                printf("Erasing...");
                gpio_put(PICO_DEFAULT_LED_PIN, false);
                // Disable interupts, erase, flash and enable interrupts
                flash_range_erase(ofs, bufsize);
                printf("  -> Flashing...\r\n");
                flash_range_program(ofs, buffer, bufsize);

                ofs += bufsize;
            } else {
                printf("Error reading rom: %d\n", fr);
                break;
            }
        }


        f_close(&fil);
        restore_interrupts(ints);
        multicore_lockout_end_blocking();
    }
}


/**
 * Function used by the rom file selector to display one page of .gb rom files
 */
uint16_t rom_file_selector_display_page(char filename[28][256], uint16_t num_page) {
    // Dirty screen cleanup
    memset(&textmode, 0x00, sizeof(textmode));
    memset(&colors, 0x00, sizeof(colors));
    char footer[80];
    sprintf(footer, "=================== PAGE #%i -> NEXT PAGE / <- PREV. PAGE ====================", num_page);
    draw_text(footer, 0, 14, 3, 11);

    DIR dj;
    FILINFO fno;
    FRESULT fr;

    fr = f_mount(&fs, "", 1);
    if (FR_OK != fr) {
        printf("E f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return 0;
    }

    /* clear the filenames array */
    for (uint8_t ifile = 0; ifile < 28; ifile++) {
        strcpy(filename[ifile], "");
    }

    /* search *.gb files */
    uint16_t num_file = 0;
    fr = f_findfirst(&dj, &fno, "SMS\\", "*");

    /* skip the first N pages */
    if (num_page > 0) {
        while (num_file < num_page * 14 && fr == FR_OK && fno.fname[0]) {
            num_file++;
            fr = f_findnext(&dj, &fno);
        }
    }

    /* store the filenames of this page */
    num_file = 0;
    while (num_file < 14 && fr == FR_OK && fno.fname[0]) {
        strcpy(filename[num_file], fno.fname);
        num_file++;
        fr = f_findnext(&dj, &fno);
    }
    f_closedir(&dj);

    /* display *.gb rom files on screen */
    // mk_ili9225_fill(0x0000);
    for (uint8_t ifile = 0; ifile < num_file; ifile++) {
        draw_text(filename[ifile], 0, ifile, 0xFF, 0x00);
    }
    return num_file;
}

/**
 * The ROM selector displays pages of up to 22 rom files
 * allowing the user to select which rom file to start
 * Copy your *.gb rom files to the root directory of the SD card
 */
void rom_file_selector() {
    uint16_t num_page = 0;
    char filenames[30][256];

    printf("Selecting ROM\r\n");

    /* display the first page with up to 22 rom files */
    uint16_t numfiles = rom_file_selector_display_page(filenames, num_page);

    /* select the first rom */
    uint8_t selected = 0;
    draw_text(filenames[selected], 0, selected, 0xFF, 0xF8);

    while (true) {
        ps2kbd.tick();
        nespad_tick();
        sleep_ms(33);
        nespad_tick();
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
        if (keyboard_bits.select || gamepad1_bits.select) {
            gpio_put(PICO_DEFAULT_LED_PIN, true);
            break;
        }

        if (keyboard_bits.start || gamepad1_bits.start || gamepad1_bits.a || gamepad1_bits.b) {
            /* copy the rom from the SD card to flash and start the game */
            char pathname[255];
            sprintf(pathname, "SMS\\%s", filenames[selected]);
            load_cart_rom_file(pathname);
            break;
        }

        if (keyboard_bits.down || gamepad1_bits.down) {
            /* select the next rom */
            draw_text(filenames[selected], 0, selected, 0xFF, 0x00);
            selected++;
            if (selected >= numfiles)
                selected = 0;
            draw_text(filenames[selected], 0, selected, 0xFF, 0xF8);
            sleep_ms(150);
        }

        if (keyboard_bits.up || gamepad1_bits.up) {
            /* select the previous rom */
            draw_text(filenames[selected], 0, selected, 0xFF, 0x00);
            if (selected == 0) {
                selected = numfiles - 1;
            } else {
                selected--;
            }
            draw_text(filenames[selected], 0, selected, 0xFF, 0xF8);
            sleep_ms(150);
        }

        if (keyboard_bits.right || gamepad1_bits.right) {
            /* select the next page */
            num_page++;
            numfiles = rom_file_selector_display_page(filenames, num_page);
            if (numfiles == 0) {
                /* no files in this page, go to the previous page */
                num_page--;
                numfiles = rom_file_selector_display_page(filenames, num_page);
            }
            /* select the first file */
            selected = 0;
            draw_text(filenames[selected], 0, selected, 0xFF, 0xF8);
            sleep_ms(150);
        }

        if ((keyboard_bits.left || gamepad1_bits.left) && num_page > 0) {
            /* select the previous page */
            num_page--;
            numfiles = rom_file_selector_display_page(filenames, num_page);
            /* select the first file */
            selected = 0;
            draw_text(filenames[selected], 0, selected, 0xFF, 0xF8);
            sleep_ms(150);
        }
        tight_loop_contents();
    }
}

static void handle_input() {
    nespad_tick();
    ps2kbd.tick();

    SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, keyboard_bits.down || gamepad1_bits.down);
    SMS_set_port_a(&sms, JOY1_UP_BUTTON, keyboard_bits.up || gamepad1_bits.up);
    SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, keyboard_bits.left || gamepad1_bits.left);
    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, keyboard_bits.right || gamepad1_bits.right);
    SMS_set_port_b(&sms, RESET_BUTTON, keyboard_bits.select || gamepad1_bits.select);
    SMS_set_port_a(&sms, JOY1_A_BUTTON, keyboard_bits.a || gamepad1_bits.a);
    SMS_set_port_a(&sms, JOY1_B_BUTTON, keyboard_bits.b || gamepad1_bits.b);
    if (is_gg) {
        SMS_set_port_b(&sms, PAUSE_BUTTON, keyboard_bits.start || gamepad1_bits.start);
    }
}

#define X2(a) (a | (a << 8))
#define X4(a) (a | (a << 8) | (a << 16) | (a << 24))
#define CHECK_BIT(var, pos) (((var)>>(pos)) & 1)

/* Renderer loop on Pico's second core */
void __time_critical_func(render_loop)() {
    multicore_lockout_victim_init();
    VgaLineBuf *linebuf;
    printf("Video on Core#%i running...\n", get_core_num());

    sem_acquire_blocking(&vga_start_semaphore);
    VgaInit(vmode, 640, 480);
    uint32_t y;
    while (linebuf = get_vga_line()) {
        y = linebuf->row;

        switch (resolution) {
            case RESOLUTION_TEXTMODE:
                for (uint8_t x = 0; x < 80; x++) {
                    uint8_t glyph_row = VGA_ROM_F16[(textmode[y / 16][x] * 16) + y % 16];
                    uint8_t color = colors[y / 16][x];

                    for (uint8_t bit = 0; bit < 8; bit++) {
                        if (CHECK_BIT(glyph_row, bit)) {
                            // FOREGROUND
                            linebuf->line[8 * x + bit] = (color >> 4) & 0xF;
                        } else {
                            // BACKGROUND
                            linebuf->line[8 * x + bit] = color & 0xF;
                        }
                    }
                }
                break;
            case RESOLUTION_NATIVE:
                if (y >= 22 && y < (22 + SMS_SCREEN_HEIGHT)) {
                    for (int x = 0; x < SMS_SCREEN_WIDTH * 2; x += 2)
                        (uint16_t &) linebuf->line[64 + x] = X2(SCREEN[(y - 22) * SMS_SCREEN_WIDTH + (x >> 1)]);
                } else {
                    memset(linebuf->line, 0, 640);
                }
#if SHOW_FPS
                // SHOW FPS
                if (y < 16) {
                    for (uint8_t x = 77; x < 80; x++) {
                        uint8_t glyph_row = VGA_ROM_F16[(textmode[y / 16][x] * 16) + y % 16];
                        uint8_t color = colors[y / 16][x];

                        for (uint8_t bit = 0; bit < 8; bit++) {
                            if (CHECK_BIT(glyph_row, bit)) {
                                // FOREGROUND
                                linebuf->line[8 * x + bit] = (color >> 4) & 0xF;
                            } else {
                                linebuf->line[8 * x + bit] = 0;
                            }
                        }
                    }
                }
#endif
        }
    }
}
// We get 6 bit RGB values, pack them into a byte swapped RGB565 value
#define VGA_RGB_222(r, g, b) ((r << 4) | (g << 2) | b)

__attribute__((always_inline)) inline uint32_t core_colour_callback(void *user, uint8_t r, uint8_t g, uint8_t b) {
//    (void) user;

    if (SMS_get_system_type(&sms) == SMS_System_GG) {
        r <<= 4;
        g <<= 4;
        b <<= 4;
    } else {
        r <<= 6;
        g <<= 6;
        b <<= 6;
    }
    return VGA_RGB_222(r >> 6, g >> 6, b >> 6);
}

uint_fast32_t frames = 0;
uint64_t start_time;
#define FRAME_SKIP (OVERCLOCKING < 366 ? 1 : 0)

static void core_vblank_callback(void *user) {
    //(void) user;
    frames++;
    static int fps_skip_counter = FRAME_SKIP;

    if (fps_skip_counter > 0) {
        fps_skip_counter--;
        SMS_skip_frame(&sms, true);
        return;
    } else {
        fps_skip_counter = FRAME_SKIP;
        SMS_skip_frame(&sms, false);
    }
}

#if ENABLE_SOUND
i2s_config_t i2s_config;
#define AUDIO_FREQ (22050)
#define SAMPLES 4096
static struct SMS_ApuSample sms_audio_samples[SAMPLES];

__attribute__((always_inline)) inline void
core_audio_callback(void *user, struct SMS_ApuSample *samples, uint32_t size) {
    (void) user;
    static int16_t audio_buffer[SAMPLES * 2];
    SMS_apu_mixer_s16(samples, reinterpret_cast<int16_t *>(audio_buffer), size);
    i2s_dma_write(&i2s_config, reinterpret_cast<const int16_t *>(audio_buffer));
}

#endif

int main() {
    /* Overclock. */
    hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
    sleep_ms(33);

    set_sys_clock_khz(OVERCLOCKING * 1000, true);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    for (int i = 0; i < 6; i++) {
        sleep_ms(33);
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(33);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }
#if !NDEBUG
    stdio_init_all();
#endif

    sleep_ms(50);
    vmode = Video(DEV_VGA, RES_HVGA);
    sleep_ms(50);

    ps2kbd.init_gpio();
    nespad_begin(clock_get_hz(clk_sys) / 1000, NES_GPIO_CLK, NES_GPIO_DATA, NES_GPIO_LAT);

    if (!SMS_init(&sms)) {
        printf("Failed to init SMS");
    }

    SMS_set_colour_callback(&sms, core_colour_callback);
    SMS_set_vblank_callback(&sms, core_vblank_callback);

#if ENABLE_SOUND
    i2s_config = i2s_get_default_config();
    i2s_config.sample_freq = AUDIO_FREQ;
    i2s_config.dma_trans_count = sizeof(sms_audio_samples) / sizeof(sms_audio_samples[0]);
    i2s_volume(&i2s_config, 0);
    i2s_init(&i2s_config);
    SMS_set_apu_callback(&sms, core_audio_callback, sms_audio_samples,
                         sizeof(sms_audio_samples) / sizeof(sms_audio_samples[0]), AUDIO_FREQ);
#endif
    SMS_set_pixels(&sms, &SCREEN, SMS_SCREEN_WIDTH, 8);

    sem_init(&vga_start_semaphore, 0, 1);
    multicore_launch_core1(render_loop);
    sem_release(&vga_start_semaphore);

    rom_file_selector();
    memset(&textmode, 0x00, sizeof(textmode));
    memset(&colors, 0x00, sizeof(colors));
    resolution = RESOLUTION_NATIVE;


    is_gg = strstr(rom_filename, ".gg") != nullptr;

    if (!SMS_loadrom(&sms, rom, rom_size, is_gg ? SMS_System_GG : SMS_System_SMS)) {
        printf("Failed running rom\r\n");
    } else {
        printf("ROM loaded %s size %i", rom_filename, rom_size);
    }

    start_time = time_us_64();

    while (true) {
        handle_input();
        SMS_run(&sms, SMS_CYCLES_PER_FRAME);
#if SHOW_FPS
        if (frames == 60) {
            uint64_t end_time;
            uint32_t diff;
            uint8_t fps;
            end_time = time_us_64();
            diff = end_time - start_time;
            fps = ((uint64_t) frames * 1000 * 1000) / diff;
            char fps_text[3];
            sprintf(fps_text, "%i", fps);
            draw_text(fps_text, 77, 0, 0xFF, 0x00);
            frames = 0;
            start_time = time_us_64();
        }
#endif
        tight_loop_contents();
    }
}
