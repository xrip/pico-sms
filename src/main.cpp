#include <cstdio>
#include <cstring>
#include <hardware/flash.h>
#include <hardware/structs/vreg_and_chip_reset.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>

#include "TotalSMS/sms.h"

#include <graphics.h>
#include "audio.h"

#include "nespad.h"
#include "ff.h"
#include "ps2kbd_mrmltr.h"

#define HOME_DIR "\\SMS"
#define FLASH_TARGET_OFFSET (1024 * 1024)
const uint8_t* rom = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

bool __uninitialized_ram(is_gg) = false;
static size_t __uninitialized_ram(rom_size) = 0;
static FATFS fs;
bool reboot = false;
bool frameskip = true;
static SMS_Core sms = { };
semaphore vga_start_semaphore;

uint8_t SCREEN[SMS_SCREEN_HEIGHT][SMS_SCREEN_WIDTH];

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

static input_bits_t keyboard_bits = { false, false, false, false, false, false, false, false };
static input_bits_t gamepad1_bits = { false, false, false, false, false, false, false, false };
static input_bits_t gamepad2_bits = { false, false, false, false, false, false, false, false };

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

static bool isInReport(hid_keyboard_report_t const* report, const unsigned char keycode) {
    for (unsigned char i: report->keycode) {
        if (i == keycode) {
            return true;
        }
    }
    return false;
}

void
__not_in_flash_func(process_kbd_report)(hid_keyboard_report_t const* report, hid_keyboard_report_t const* prev_report) {
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

static void handle_input() {

    SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, keyboard_bits.down || gamepad1_bits.down);
    SMS_set_port_a(&sms, JOY1_UP_BUTTON, keyboard_bits.up || gamepad1_bits.up);
    SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, keyboard_bits.left || gamepad1_bits.left);
    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, keyboard_bits.right || gamepad1_bits.right);
    // SMS_set_port_b(&sms, RESET_BUTTON, keyboard_bits.select || gamepad1_bits.select);
    SMS_set_port_a(&sms, JOY1_A_BUTTON, keyboard_bits.a || gamepad1_bits.a);
    SMS_set_port_a(&sms, JOY1_B_BUTTON, keyboard_bits.b || gamepad1_bits.b);

    if (is_gg) {
        SMS_set_port_b(&sms, PAUSE_BUTTON, keyboard_bits.start || gamepad1_bits.start);
    }
}


__attribute__((always_inline)) inline uint32_t core_colour_callback(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {

    if (SMS_get_system_type(&sms) == SMS_System_GG) {
        r <<= 4;
        g <<= 4;
        b <<= 4;
    }
    else {
        r <<= 6;
        g <<= 6;
        b <<= 6;
    }
    graphics_set_palette(index, RGB888(r,g,b));
    return index;
}

uint_fast32_t frames = 0;
uint64_t start_time;

static __always_inline void core_vblank_callback(void* user) {
    //(void) user;
    frames++;
    static int fps_skip_counter = frameskip ? 0 : 1;

    if (fps_skip_counter > 0) {
        fps_skip_counter--;
        SMS_skip_frame(&sms, true);
    } else {
        fps_skip_counter = frameskip ? 0 : 1;
        SMS_skip_frame(&sms, false);
    }
}

i2s_config_t i2s_config;
#define AUDIO_FREQ (22050)
#define SAMPLES 4096
static SMS_ApuSample sms_audio_samples[SAMPLES];

static void core_audio_callback(void* user, SMS_ApuSample* samples, uint32_t size) {
    static int16_t audio_buffer[SAMPLES * 2];
    SMS_apu_mixer_s16(samples, audio_buffer, size);
    i2s_dma_write(&i2s_config, audio_buffer);
}

typedef struct __attribute__((__packed__)) {
    bool is_directory;
    bool is_executable;
    size_t size;
    char filename[79];
} file_item_t;

constexpr int max_files = 600;
file_item_t * fileItems = (file_item_t *)(&SCREEN[0][0] + TEXTMODE_COLS*TEXTMODE_ROWS*2);

int compareFileItems(const void* a, const void* b) {
    const auto* itemA = (file_item_t *)a;
    const auto* itemB = (file_item_t *)b;
    // Directories come first
    if (itemA->is_directory && !itemB->is_directory)
        return -1;
    if (!itemA->is_directory && itemB->is_directory)
        return 1;
    // Sort files alphabetically
    return strcmp(itemA->filename, itemB->filename);
}

bool isExecutable(const char pathname[255],const char *extensions) {
    char *pathCopy = strdup(pathname);
    const char* token = strtok(pathCopy, ".");
    while (token != NULL) {
        if (strstr(extensions, token) != NULL) {
            free(pathCopy);
            return true;
        }
        token = strtok(NULL, ",");
    }
    free(pathCopy);
    return false;
}

bool filebrowser_loadfile(const char pathname[256]) {
    UINT bytes_read = 0;
    FIL file;

    constexpr int window_y = (TEXTMODE_ROWS - 5) / 2;
    constexpr int window_x = (TEXTMODE_COLS - 43) / 2;

    draw_window("Loading ROM", window_x, window_y, 43, 5);

    FILINFO fileinfo;
    f_stat(pathname, &fileinfo);
    rom_size = fileinfo.fsize;
    if (16384 - 64 << 10 < fileinfo.fsize) {
        draw_text("ERROR: ROM too large! Canceled!!", window_x + 1, window_y + 2, 13, 1);
        sleep_ms(5000);
        return false;
    }


    draw_text("Loading...", window_x + 1, window_y + 2, 10, 1);
    sleep_ms(500);


    multicore_lockout_start_blocking();
    auto flash_target_offset = FLASH_TARGET_OFFSET;
    const uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(flash_target_offset, fileinfo.fsize);
    restore_interrupts(ints);

    if (FR_OK == f_open(&file, pathname, FA_READ)) {
        uint8_t buffer[FLASH_PAGE_SIZE];

        do {
            f_read(&file, &buffer, FLASH_PAGE_SIZE, &bytes_read);

            if (bytes_read) {
                const uint32_t ints = save_and_disable_interrupts();
                flash_range_program(flash_target_offset, buffer, FLASH_PAGE_SIZE);
                restore_interrupts(ints);

                gpio_put(PICO_DEFAULT_LED_PIN, flash_target_offset >> 13 & 1);

                flash_target_offset += FLASH_PAGE_SIZE;
            }
        }
        while (bytes_read != 0);

        gpio_put(PICO_DEFAULT_LED_PIN, true);
    }
    f_close(&file);
    multicore_lockout_end_blocking();
    // restore_interrupts(ints);

    is_gg = strstr(pathname, ".gg") != nullptr;
    return true;
}

void filebrowser(const char pathname[256], const char executables[11]) {
    bool debounce = true;
    char basepath[256];
    char tmp[TEXTMODE_COLS + 1];
    strcpy(basepath, pathname);
    constexpr int per_page = TEXTMODE_ROWS - 3;

    DIR dir;
    FILINFO fileInfo;

    if (FR_OK != f_mount(&fs, "SD", 1)) {
        draw_text("SD Card not inserted or SD Card error!", 0, 0, 12, 0);
        while (true);
    }

    while (true) {
        memset(fileItems, 0, sizeof(file_item_t) * max_files);
        int total_files = 0;

        snprintf(tmp, TEXTMODE_COLS, "SD:\\%s", basepath);
        draw_window(tmp, 0, 0, TEXTMODE_COLS, TEXTMODE_ROWS - 1);
        memset(tmp, ' ', TEXTMODE_COLS);


        draw_text(tmp, 0, 29, 0, 0);
        auto off = 0;
        draw_text("START", off, 29, 7, 0);
        off += 5;
        draw_text(" Run at cursor ", off, 29, 0, 3);
        off += 16;
        draw_text("SELECT", off, 29, 7, 0);
        off += 6;
        draw_text(" Run previous  ", off, 29, 0, 3);
#ifndef TFT
        off += 16;
        draw_text("ARROWS", off, 29, 7, 0);
        off += 6;
        draw_text(" Navigation    ", off, 29, 0, 3);
        off += 16;
        draw_text("A/F10", off, 29, 7, 0);
        off += 5;
        draw_text(" USB DRV ", off, 29, 0, 3);
#endif

        if (FR_OK != f_opendir(&dir, basepath)) {
            draw_text("Failed to open directory", 1, 1, 4, 0);
            while (true);
        }

        if (strlen(basepath) > 0) {
            strcpy(fileItems[total_files].filename, "..\0");
            fileItems[total_files].is_directory = true;
            fileItems[total_files].size = 0;
            total_files++;
        }

        while (f_readdir(&dir, &fileInfo) == FR_OK &&
               fileInfo.fname[0] != '\0' &&
               total_files < max_files
        ) {
            // Set the file item properties
            fileItems[total_files].is_directory = fileInfo.fattrib & AM_DIR;
            fileItems[total_files].size = fileInfo.fsize;
            fileItems[total_files].is_executable = isExecutable(fileInfo.fname, executables);
            strncpy(fileItems[total_files].filename, fileInfo.fname, 78);
            total_files++;
        }
        f_closedir(&dir);

        qsort(fileItems, total_files, sizeof(file_item_t), compareFileItems);

        if (total_files > max_files) {
            draw_text(" Too many files!! ", TEXTMODE_COLS - 17, 0, 12, 3);
        }

        int offset = 0;
        int current_item = 0;

        while (true) {
            sleep_ms(100);

            if (!debounce) {
                debounce = !(nespad_state & DPAD_START || keyboard_bits.start);
            }

            // ESCAPE
            if (nespad_state & DPAD_SELECT || keyboard_bits.select) {
                return;
            }

            if (nespad_state & DPAD_DOWN || keyboard_bits.down) {
                if (offset + (current_item + 1) < total_files) {
                    if (current_item + 1 < per_page) {
                        current_item++;
                    }
                    else {
                        offset++;
                    }
                }
            }

            if (nespad_state & DPAD_UP || keyboard_bits.up) {
                if (current_item > 0) {
                    current_item--;
                }
                else if (offset > 0) {
                    offset--;
                }
            }

            if (nespad_state & DPAD_RIGHT || keyboard_bits.right) {
                offset += per_page;
                if (offset + (current_item + 1) > total_files) {
                    offset = total_files - (current_item + 1);
                }
            }

            if (nespad_state & DPAD_LEFT || keyboard_bits.left) {
                if (offset > per_page) {
                    offset -= per_page;
                }
                else {
                    offset = 0;
                    current_item = 0;
                }
            }

            if (debounce && (nespad_state & DPAD_START || keyboard_bits.start)) {
                auto file_at_cursor = fileItems[offset + current_item];

                if (file_at_cursor.is_directory) {
                    if (strcmp(file_at_cursor.filename, "..") == 0) {
                        const char* lastBackslash = strrchr(basepath, '\\');
                        if (lastBackslash != nullptr) {
                            const size_t length = lastBackslash - basepath;
                            basepath[length] = '\0';
                        }
                    }
                    else {
                        sprintf(basepath, "%s\\%s", basepath, file_at_cursor.filename);
                    }
                    debounce = false;
                    break;
                }

                if (file_at_cursor.is_executable) {
                    sprintf(tmp, "%s\\%s", basepath, file_at_cursor.filename);

                    filebrowser_loadfile(tmp);
                    return;
                }
            }

            for (int i = 0; i < per_page; i++) {
                uint8_t color = 11;
                uint8_t bg_color = 1;

                if (offset + i < max_files) {
                    const auto item = fileItems[offset + i];


                    if (i == current_item) {
                        color = 0;
                        bg_color = 3;
                        memset(tmp, 0xCD, TEXTMODE_COLS - 2);
                        tmp[TEXTMODE_COLS - 2] = '\0';
                        draw_text(tmp, 1, per_page + 1, 11, 1);
                        snprintf(tmp, TEXTMODE_COLS - 2, " Size: %iKb, File %lu of %i ", item.size / 1024,
                                 offset + i + 1,
                                 total_files);
                        draw_text(tmp, 2, per_page + 1, 14, 3);
                    }

                    const auto len = strlen(item.filename);
                    color = item.is_directory ? 15 : color;
                    color = item.is_executable ? 10 : color;
                    //color = strstr((char *)rom_filename, item.filename) != nullptr ? 13 : color;

                    memset(tmp, ' ', TEXTMODE_COLS - 2);
                    tmp[TEXTMODE_COLS - 2] = '\0';
                    memcpy(&tmp, item.filename, len < TEXTMODE_COLS - 2 ? len : TEXTMODE_COLS - 2);
                }
                else {
                    memset(tmp, ' ', TEXTMODE_COLS - 2);
                }
                draw_text(tmp, 1, i + 1, color, bg_color);
            }
        }
    }
}

enum menu_type_e {
    EMPTY,
    INT,
    TEXT,
    ARRAY,

    OVERCLOCK,

    SAVE,
    LOAD,
    ROM_SELECT,
    RETURN,
};

typedef struct __attribute__((__packed__)) {
    const char *text;
    menu_type_e type;
    const void *value;
    uint8_t max_value;
    char value_list[5][10];
} MenuItem;

uint16_t overclock[3] = { 378, 396, 416 };
uint8_t overclock_v = 0;

const MenuItem menu_items[] = {
        // {"Player 1: %s",        ARRAY, &player_1_input, 2, {"Keyboard ", "Gamepad 1", "Gamepad 2"}},
        {"Overclocking: %s",        OVERCLOCK, &overclock_v, 2, {"378 Mhz", "396 Mhz", "416 Mhz"}},
    //{"Player 2: %s",        ARRAY, &player_2_input, 2, {"Keyboard ", "Gamepad 1", "Gamepad 2"}},
        {""},
        {"Frameskip: %s",     ARRAY, &frameskip,  1, {"NO ",       "YES"}},
        // {"Limit fps: %s",     ARRAY, &limit_fps,    1, {"NO ",       "YES"}},
        //{"Show fps: %s",     ARRAY, &show_fps,    1, {"NO ",       "YES"}},
        {""},
        {"Reset to ROM select", ROM_SELECT},
        {"Return to game",      RETURN}
};

#define MENU_ITEMS_NUMBER (sizeof(menu_items) / sizeof (MenuItem))

void menu() {
    bool exit = false;
    graphics_set_mode(TEXTMODE_DEFAULT);
    char footer[TEXTMODE_COLS];
    snprintf(footer, TEXTMODE_COLS, ":: %s ::", PICO_PROGRAM_NAME);
    draw_text(footer, TEXTMODE_COLS / 2 - strlen(footer) / 2, 0, 11, 1);
    snprintf(footer, TEXTMODE_COLS, ":: %s build %s %s ::", PICO_PROGRAM_VERSION_STRING, __DATE__,
             __TIME__);
    draw_text(footer, TEXTMODE_COLS / 2 - strlen(footer) / 2, TEXTMODE_ROWS - 1, 11, 1);
    uint current_item = 0;

    while (!exit) {
        sleep_ms(25);
        if (gamepad1_bits.down || keyboard_bits.down) {
            current_item = (current_item + 1) % MENU_ITEMS_NUMBER;
            if (menu_items[current_item].type == EMPTY)
                current_item++;
        }
        if (gamepad1_bits.up || keyboard_bits.up) {
            current_item = (current_item - 1 + MENU_ITEMS_NUMBER) % MENU_ITEMS_NUMBER;
            if (menu_items[current_item].type == EMPTY)
                current_item--;
        }
        for (int i = 0; i < MENU_ITEMS_NUMBER; i++) {
            uint8_t y = i + (TEXTMODE_ROWS - MENU_ITEMS_NUMBER >> 1);
            uint8_t x = TEXTMODE_COLS / 2 - 10;
            uint8_t color = 0xFF;
            uint8_t bg_color = 0x00;
            if (current_item == i) {
                color = 0x01;
                bg_color = 0xFF;
            }
            const MenuItem* item = &menu_items[i];
            if (i == current_item) {
                switch (item->type) {
                    case INT:
                    case ARRAY:
                        if (item->max_value != 0) {
                            auto* value = (uint8_t *)item->value;
                            if ((gamepad1_bits.right || keyboard_bits.right) && *value < item->max_value) {
                                (*value)++;
                            }
                            if ((gamepad1_bits.left || keyboard_bits.left) && *value > 0) {
                                (*value)--;
                            }
                        }
                        break;
                    case OVERCLOCK:
                        if (item->max_value != 0) {
                            auto* value = (uint8_t *)item->value;
                            if ((gamepad1_bits.right || keyboard_bits.right) && *value < item->max_value) {
                                (*value)++;
                                hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
                                sleep_ms(10);
                                set_sys_clock_khz(overclock[overclock_v] * KHZ, true);
                            }
                            if ((gamepad1_bits.left || keyboard_bits.left) && *value > 0) {
                                (*value)--;
                                hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
                                sleep_ms(10);
                                set_sys_clock_khz(overclock[overclock_v] * KHZ, true);

                            }
                        }
                        break;
                    case SAVE:
                        if (gamepad1_bits.start || keyboard_bits.start) {
                            // save();
                            exit = true;
                        }
                        break;
                    case LOAD:
                        if (gamepad1_bits.start || keyboard_bits.start) {
                            // load();
                            exit = true;
                        }
                        break;
                    case RETURN:
                        if (gamepad1_bits.start || keyboard_bits.start)
                            exit = true;
                        break;

                    case ROM_SELECT:
                        if (gamepad1_bits.start || keyboard_bits.start) {
                            reboot = true;
                            return;
                        }
                        break;
                }
            }
            static char result[TEXTMODE_COLS];
            switch (item->type) {
                case INT:
                    snprintf(result, TEXTMODE_COLS, item->text, *(uint8_t *)item->value);
                    break;
                case ARRAY:
                case OVERCLOCK:
                    snprintf(result, TEXTMODE_COLS, item->text, item->value_list[*(uint8_t *)item->value]);
                    break;
                case TEXT:
                    snprintf(result, TEXTMODE_COLS, item->text, item->value);
                    break;
                default:
                    snprintf(result, TEXTMODE_COLS, "%s", item->text);
            }
            draw_text(result, x, y, color, bg_color);
        }
        sleep_ms(100);
    }

    graphics_set_mode(GRAPHICSMODE_DEFAULT);
}

/* Renderer loop on Pico's second core */
void __scratch_x("render") render_core() {
    multicore_lockout_victim_init();

    ps2kbd.init_gpio();
    nespad_begin(clock_get_hz(clk_sys) / 1000, NES_GPIO_CLK, NES_GPIO_DATA, NES_GPIO_LAT);


    i2s_config = i2s_get_default_config();
    i2s_config.sample_freq = AUDIO_FREQ;
    i2s_config.dma_trans_count = sizeof(sms_audio_samples) / sizeof(sms_audio_samples[0]);
    i2s_volume(&i2s_config, 0);
    i2s_init(&i2s_config);
    SMS_set_apu_callback(&sms, core_audio_callback, sms_audio_samples,
                         sizeof(sms_audio_samples) / sizeof(sms_audio_samples[0]), AUDIO_FREQ);
    SMS_set_colour_callback(&sms, core_colour_callback);
    SMS_set_vblank_callback(&sms, core_vblank_callback);

    graphics_init();

    const auto buffer = (uint8_t *)SCREEN;
    graphics_set_buffer(buffer, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);
    graphics_set_textbuffer(buffer);
    graphics_set_bgcolor(0x000000);
    graphics_set_offset(32, 24);

    graphics_set_flashmode(false, false);
    sem_acquire_blocking(&vga_start_semaphore);

    // 60 FPS loop
#define frame_tick (16666)
    uint64_t tick = time_us_64();
#ifdef TFT
    uint64_t last_renderer_tick = tick;
#endif
    uint64_t last_input_tick = tick;
    while (true) {
#ifdef TFT
        if (tick >= last_renderer_tick + frame_tick) {
            refresh_lcd();
            last_renderer_tick = tick;
        }
#endif
        if (tick >= last_input_tick + frame_tick * 1) {
            ps2kbd.tick();
            nespad_tick();
            last_input_tick = tick;
        }
        tick = time_us_64();


        // tuh_task();
        //hid_app_task();
        tight_loop_contents();
    }

    __unreachable();
}

int main() {
    hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
    sleep_ms(10);
    set_sys_clock_khz(378 * KHZ, true);

    SMS_init(&sms);

    sem_init(&vga_start_semaphore, 0, 1);
    multicore_launch_core1(render_core);
    sem_release(&vga_start_semaphore);


    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    for (int i = 0; i < 6; i++) {
        sleep_ms(33);
        gpio_put(PICO_DEFAULT_LED_PIN, true);
        sleep_ms(33);
        gpio_put(PICO_DEFAULT_LED_PIN, false);
    }


    SMS_set_pixels(&sms, &SCREEN, SMS_SCREEN_WIDTH, 8);

    while (true) {
        graphics_set_mode(TEXTMODE_DEFAULT);
        filebrowser(HOME_DIR, "sms,gg");

        if (!SMS_loadrom(&sms, rom, rom_size, is_gg ? SMS_System_GG : SMS_System_SMS)) {
            while (1) draw_text("Failed running rom!", 0, 0, 4, 0);
        }

        graphics_set_mode(GRAPHICSMODE_DEFAULT);


        start_time = time_us_64();

        while (!reboot) {
            handle_input();
            SMS_run(&sms, SMS_CYCLES_PER_FRAME);

            if ((gamepad1_bits.start && gamepad1_bits.select) || (keyboard_bits.start && keyboard_bits.select)) {
                menu();
            }

            tight_loop_contents();
        }

        reboot = false;
    }
    __unreachable();
}
