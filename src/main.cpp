#include <cstdio>
#include <cstring>
#include <hardware/flash.h>
#include <hardware/structs/vreg_and_chip_reset.h>
#include <pico/multicore.h>
#include <pico/stdlib.h>

#include "../TotalSMS/sms.h"

#include <graphics.h>
#include "audio.h"

#include "nespad.h"
#include "ff.h"
#include "ps2kbd_mrmltr.h"

#define HOME_DIR "\\SMS"
#define FLASH_TARGET_OFFSET (1024 * 1024)
const uint8_t* rom = (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

bool __uninitialized_ram(is_gg) = false;
char __uninitialized_ram(filename[256]);
static size_t __uninitialized_ram(rom_size) = 0;

static FATFS fs;
bool reboot = false;
bool frameskip = true;
bool limit_fps = true;
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

bool swap_ab = false;

void nespad_tick() {
    nespad_read();

    if (swap_ab) {
        gamepad1_bits.a = (nespad_state & DPAD_A) != 0;
        gamepad1_bits.b = (nespad_state & DPAD_B) != 0;
    } else {
        gamepad1_bits.b = (nespad_state & DPAD_A) != 0;
        gamepad1_bits.a = (nespad_state & DPAD_B) != 0;
    }
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
    const char* token = strrchr(pathCopy, '.');

    if (token == nullptr) {
        return false;
    }

    token++;

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

    strcpy(filename, fileinfo.fname);

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
    NONE,
    INT,
    TEXT,
    ARRAY,

    SAVE,
    LOAD,
    ROM_SELECT,
    RETURN,
};

typedef bool (*menu_callback_t)();

typedef struct __attribute__((__packed__)) {
    const char* text;
    menu_type_e type;
    const void* value;
    menu_callback_t callback;
    uint8_t max_value;
    char value_list[15][10];
} MenuItem;

int save_slot = 0;
uint16_t frequencies[] = { 378, 396, 404, 408, 412, 416, 420, 424, 432 };
uint8_t frequency_index = 0;

bool overclock() {
    hw_set_bits(&vreg_and_chip_reset_hw->vreg, VREG_AND_CHIP_RESET_VREG_VSEL_BITS);
    sleep_ms(10);
    return set_sys_clock_khz(frequencies[frequency_index] * KHZ, true);
}

bool save() {
    char pathname[255];

    if (save_slot) {
        sprintf(pathname, "SMS\\%s_%d.save", filename, save_slot);
    }
    else {
        sprintf(pathname, "SMS\\%s.save", filename);
    }

    FRESULT fr = f_mount(&fs, "", 1);
    FIL fd;
    fr = f_open(&fd, pathname, FA_CREATE_ALWAYS | FA_WRITE);
    UINT br;

    f_write(&fd, &sms.cpu, sizeof(sms.cpu), &br);
    f_write(&fd, &sms.vdp, sizeof(sms.vdp), &br);
    f_write(&fd, &sms.psg, sizeof(sms.psg), &br);

    f_write(&fd, &sms.cart, sizeof(sms.cart), &br);
    f_write(&fd, &sms.memory_control, sizeof(sms.memory_control), &br);
    f_write(&fd, &sms.system_ram, sizeof(sms.system_ram), &br);

    f_close(&fd);

    return true;
}

bool load() {
    char pathname[255];

    if (save_slot) {
        sprintf(pathname, "SMS\\%s_%d.save", filename, save_slot);
    }
    else {
        sprintf(pathname, "SMS\\%s.save", filename);
    }

    FRESULT fr = f_mount(&fs, "", 1);
    FIL fd;
    fr = f_open(&fd, pathname, FA_READ);
    UINT br;

    f_read(&fd, &sms.cpu, sizeof(sms.cpu), &br);
    f_read(&fd, &sms.vdp, sizeof(sms.vdp), &br);
    f_read(&fd, &sms.psg, sizeof(sms.psg), &br);

    f_read(&fd, &sms.cart, sizeof(sms.cart), &br);
    f_read(&fd, &sms.memory_control, sizeof(sms.memory_control), &br);
    f_read(&fd, &sms.system_ram, sizeof(sms.system_ram), &br);

    f_close(&fd);

    // FIXME remove
    SMS_loadstate(&sms, nullptr);

    return true;
}


const MenuItem menu_items[] = {
    {"Swap AB <> BA: %s",     ARRAY, &swap_ab,  nullptr, 1, {"NO ",       "YES"}},
    {"Frameskip: %s",     ARRAY, &frameskip,  nullptr, 1, {"YES",       "NO "}},
    //{ "Player 1: %s",        ARRAY, &player_1_input, 2, { "Keyboard ", "Gamepad 1", "Gamepad 2" }},
    //{ "Player 2: %s",        ARRAY, &player_2_input, 2, { "Keyboard ", "Gamepad 1", "Gamepad 2" }},
    {},
    { "Save state: %i", INT, &save_slot, &save, 5 },
    { "Load state: %i", INT, &save_slot, &load, 5 },
{},
{
    "Overclocking: %s MHz", ARRAY, &frequency_index, &overclock, count_of(frequencies) - 1,
    { "378", "396", "404", "408", "412", "416", "420", "424", "432" }
},
{ "Press START / Enter to apply", NONE },
    { "Reset to ROM select", ROM_SELECT },
    { "Return to game", RETURN }
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
                    default:
                        break;
                }

                if (nullptr != item->callback && (gamepad1_bits.start || keyboard_bits.start)) {
                    exit = item->callback();
                }
            }
            static char result[TEXTMODE_COLS];
            switch (item->type) {
                case INT:
                    snprintf(result, TEXTMODE_COLS, item->text, *(uint8_t *)item->value);
                    break;
                case ARRAY:
                    snprintf(result, TEXTMODE_COLS, item->text, item->value_list[*(uint8_t *)item->value]);
                    break;
                case TEXT:
                    snprintf(result, TEXTMODE_COLS, item->text, item->value);
                    break;
                case NONE:
                    color = 6;
                default:
                    snprintf(result, TEXTMODE_COLS, "%s", item->text);
            }
            draw_text(result, x, y, color, bg_color);
        }

        if (gamepad1_bits.down || keyboard_bits.down) {
            current_item = (current_item + 1) % MENU_ITEMS_NUMBER;

            if (menu_items[current_item].type == NONE)
                current_item++;
        }
        if (gamepad1_bits.up || keyboard_bits.up) {
            current_item = (current_item - 1 + MENU_ITEMS_NUMBER) % MENU_ITEMS_NUMBER;

            if (menu_items[current_item].type == NONE)
                current_item--;
        }

        sleep_ms(125);
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
    uint64_t last_frame_tick = tick;

    while (true) {

        if (tick >= last_frame_tick + frame_tick) {
#ifdef TFT
            refresh_lcd();
#endif
            ps2kbd.tick();
            nespad_tick();

            last_frame_tick = tick;
        }

        tick = time_us_64();

        // tuh_task();
        // hid_app_task();
        tight_loop_contents();
    }

    __unreachable();
}

int frame, frame_cnt = 0;
int frame_timer_start = 0;

int main() {
    overclock();

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


            frame++;
            if (limit_fps) {

                frame_cnt++;
                if (frame_cnt == 6) {
                    while (time_us_64() - frame_timer_start < 16666 * 6);  // 60 Hz
                    frame_timer_start = time_us_64();
                    frame_cnt = 0;
                }
            }
            tight_loop_contents();
        }

        reboot = false;
    }
    __unreachable();
}
