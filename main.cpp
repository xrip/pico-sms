#include <cstdio>
#include "pico/stdlib.h"
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
#if ENABLE_SOUND
#include "audio.h"
#endif

#include "nespad.h"
#include "f_util.h"
#include "ff.h"
#include "VGA_ROM_F16.h"

/*
#define ROM Sonic_The_Hedgehog__USA__Europe__sms
#define ROM_SIZE Sonic_The_Hedgehog__USA__Europe__sms_len
#define ROM_NAME "rom.sms"
*/
#define FLASH_TARGET_OFFSET (1024 * 1024)
const uint8_t *rom = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
unsigned int rom_size = 0;
static FATFS fs;

static struct SMS_Core sms = { 0 };
static const sVmode *vmode = nullptr;
struct semaphore vga_start_semaphore;

uint8_t SCREEN[SMS_SCREEN_WIDTH * SMS_SCREEN_HEIGHT];
char textmode[30][80];
uint8_t colors[30][80];

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
    UINT br = 0;
    uint8_t buffer[FLASH_SECTOR_SIZE];
    bool mismatch = false;

    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr) {
        printf("E f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return;
    }
    FIL fil;
    FILINFO fi;
    f_stat(filename, &fi);
    rom_size = fi.fsize;
    fr = f_open(&fil, filename, FA_READ);

    if (fr == FR_OK) {
        multicore_lockout_start_blocking();
        uint32_t ints = save_and_disable_interrupts();
        uint32_t flash_target_offset = FLASH_TARGET_OFFSET;
        for (;;) {
            f_read(&fil, buffer, sizeof buffer, &br);
            if (br == 0)
                break; /* end of file */


            printf("I Erasing target region...\n");
            flash_range_erase(flash_target_offset, FLASH_SECTOR_SIZE);
            printf("I Programming target region...\n");
            flash_range_program(flash_target_offset, buffer, FLASH_SECTOR_SIZE);
            /* Read back target region and check programming */
            printf("I Done. Reading back target region...\n");
            for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i++) {
                if (rom[flash_target_offset + i] != buffer[i]) {
                    mismatch = true;
                }
            }

            /* Next sector */
            flash_target_offset += FLASH_SECTOR_SIZE;
        }
        restore_interrupts(ints);
        multicore_lockout_end_blocking();
        if (mismatch) {
            printf("I Programming successful!\n");
        } else {
            printf("E Programming failed!\n");
        }
    } else {
        printf("E f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    }

    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("E f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }

    printf("I load_cart_rom_file(%s) COMPLETE (%u bytes)\n", filename, br);
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
    draw_text(footer, 0, 29, 3, 11);

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
    fr = f_findfirst(&dj, &fno, "SMS\\", "*.sms");

    /* skip the first N pages */
    if (num_page > 0) {
        while (num_file < num_page * 28 && fr == FR_OK && fno.fname[0]) {
            num_file++;
            fr = f_findnext(&dj, &fno);
        }
    }

    /* store the filenames of this page */
    num_file = 0;
    while (num_file < 28 && fr == FR_OK && fno.fname[0]) {
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


        nespad_read();
        sleep_ms(100);
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
        if ((nespad_state & 0x04) != 0 || (nespad_state & 0x01) != 0 || (nespad_state & 0x02) != 0) {
            /* copy the rom from the SD card to flash and start the game */
            char pathname[255];
            sprintf(pathname, "SMS\\%s", filenames[selected]);
            load_cart_rom_file(pathname);
            break;
        }
        if ((nespad_state & 0x20) != 0) {
            /* select the next rom */
            draw_text(filenames[selected], 0, selected, 0xFF, 0x00);
            selected++;
            if (selected >= numfiles)
                selected = 0;
            draw_text(filenames[selected], 0, selected, 0xFF, 0xF8);
            sleep_ms(150);
        }
        if ((nespad_state & 0x10) != 0) {
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
        if ((nespad_state & 0x80) != 0) {
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
        if ((nespad_state & 0x40) != 0 && num_page > 0) {
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


#define X2(a) (a | (a << 8))
#define CHECK_BIT(var, pos) (((var)>>(pos)) & 1)

/* Renderer loop on Pico's second core */
void __time_critical_func(render_loop)() {
    multicore_lockout_victim_init();
    VgaLineBuf *linebuf;
    printf("Video on Core#%i running...\n", get_core_num());

    sem_acquire_blocking(&vga_start_semaphore);
    VgaInit(vmode, 640, 480);

    while (linebuf = get_vga_line()) {
        uint32_t y = linebuf->row;

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
                if (y >= 44 && y < (44 + SMS_SCREEN_HEIGHT * 2)) {
                    for (int x = 0; x < SMS_SCREEN_WIDTH * 2; x += 2)
                        (uint16_t &) linebuf->line[64 + x] = X2(SCREEN[((y - 44) >> 1) * SMS_SCREEN_WIDTH + (x >> 1)]);
                } else {
                    memset(linebuf->line, 0, 640);
                }
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
#define FRAME_SKIP (1)
static void core_vblank_callback(void *user) {
    (void) user;
    frames++;
    static int fps_skip_counter = FRAME_SKIP;

    if (fps_skip_counter > 0)
    {
        fps_skip_counter--;
        SMS_skip_frame(&sms, true);
        return;
    }
    else
    {
        fps_skip_counter = FRAME_SKIP;
        SMS_skip_frame(&sms, false);
    }

}

static void handle_input() {
    nespad_read();


    SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, (nespad_state & 0x20) != 0);
    SMS_set_port_a(&sms, JOY1_UP_BUTTON, (nespad_state & 0x10) != 0);
    SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, (nespad_state & 0x40) != 0);
    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON,  (nespad_state & 0x80) != 0);
    SMS_set_port_a(&sms, JOY1_A_BUTTON, (nespad_state & 0x01) != 0);
    SMS_set_port_a(&sms, JOY1_B_BUTTON, (nespad_state & 0x02) != 0);
    SMS_set_port_b(&sms, RESET_BUTTON, (nespad_state & 0x04) != 0);
    SMS_set_port_b(&sms, PAUSE_BUTTON, (nespad_state & 0x08) != 0);
}


#if ENABLE_SOUND
i2s_config_t i2s_config;
#define AUDIO_FREQ (22050)
#define SAMPLES 4096
static struct SMS_ApuSample sms_audio_samples[SAMPLES];

__attribute__((always_inline)) inline void core_audio_callback(void* user, struct SMS_ApuSample* samples, uint32_t size) {
    (void)user;
    static int_fast16_t audio_buffer[SAMPLES*2];
    SMS_apu_mixer_s16(samples, reinterpret_cast<int16_t *>(audio_buffer), size);
    i2s_dma_write(&i2s_config, reinterpret_cast<const int16_t *>(audio_buffer));
}
#endif

int main() {
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    set_sys_clock_khz(282000, true);

    //stdio_init_all();

//    sleep_ms(3000);
    nespad_begin(clock_get_hz(clk_sys) / 1000, NES_GPIO_CLK, NES_GPIO_DATA, NES_GPIO_LAT);
    printf("Start program\n");

    sleep_ms(50);
    vmode = Video(DEV_VGA, RES_VGA);
    sleep_ms(50);

    // util::dumpMemory((void *)NES_FILE_ADDR, 1024);
    sem_init(&vga_start_semaphore, 0, 1);
    multicore_launch_core1(render_loop);
    sem_release(&vga_start_semaphore);

#if ENABLE_SOUND
    i2s_config = i2s_get_default_config();
    i2s_config.sample_freq = AUDIO_FREQ;
    i2s_config.dma_trans_count = i2s_config.sample_freq / 20;
    i2s_volume(&i2s_config, 0);
    i2s_init(&i2s_config);
#endif
    rom_file_selector();
    memset(&textmode, 0x00, sizeof(textmode));
    memset(&colors, 0x00, sizeof(colors));
    sleep_ms(50);
    resolution = RESOLUTION_NATIVE;

    if (!SMS_init(&sms)) {
        printf("Failed to init SMS");
    }

    SMS_set_colour_callback(&sms, core_colour_callback);
    SMS_set_vblank_callback(&sms, core_vblank_callback);

#if ENABLE_SOUND
    SMS_set_apu_callback(&sms, core_audio_callback, sms_audio_samples, sizeof(sms_audio_samples)/sizeof(sms_audio_samples[0]), AUDIO_FREQ);
#endif
    SMS_set_pixels(&sms, &SCREEN, SMS_SCREEN_WIDTH, 8);

    //mgb_init(&sms);

    if (!SMS_loadrom(&sms, rom, rom_size, SMS_System_SMS)) {
        printf("Failed running rom\r\n");
    } else {
            printf("ROM loaded.  size %i", rom_size);
    };

    start_time = time_us_64();

    for (;;) {
        handle_input();
        SMS_run(&sms, SMS_CYCLES_PER_FRAME);

/*        if (frames == 600) {
            uint64_t end_time;
            uint32_t diff;
            uint32_t fps;

            end_time = time_us_64();
            diff = end_time - start_time;
            fps = ((uint64_t) frames * 1000 * 1000) / diff;
            printf("Frames: %u\r\n"
                   "Time: %lu us\r\n"
                   "FPS: %lu\r\n",
                   frames, diff, fps);
            stdio_flush();
            frames = 0;
            start_time = time_us_64();
        }*/

    }
}
