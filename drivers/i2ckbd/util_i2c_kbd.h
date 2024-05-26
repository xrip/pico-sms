#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "inttypes.h"
#include "kb_u_codes.h"


#define i2c_port (i2c0)
#define CLOCK_I2C_kHz (400)
#define I2C_ADDRESS (0x20)
#define PICO_I2C_SDA_PIN (0)
#define PICO_I2C_SCL_PIN (1)
#define MAX_QNT_DEVICES (8)
#define KEY_ARRAY_SIZE (12)

#define    I2C_40_EXT_KEYBOARD (0x20)
#define    I2C_STRIKE_KEYBOARD (0x21)
#define    I2C_MC7007_KEYBOARD (0x22)
#define    I2C_RP2040USB_KEYBOARD (0x77)


#define KEY_ARRAY_SIZE_RP2040 (26)
#define I2C_KEY_NOT_PRESSED (0)
#define I2C_KEY_PRESSED_TIMEOUT (350*1000)
#define I2C_KEY_REPEAT_TIMEOUT (100*1000)
#define    I2C_NEGATIVE_SCAN (0xFE)
#define I2C_POSITIVE_SCAN (0x01)

typedef struct kbd_markup kbd_markup;
struct kbd_markup {
    uint8_t i2c_dev;    // Индекс устройства i2c
    uint8_t i2c_idx;    // Индекс порта i2c (адрес байта в массиве)
    uint8_t i2c_mask;    // Маска сканкода i2c
    uint8_t kbd_idx;    // Индекс порта PS/2 (адрес байта в массиве)
    uint32_t kbd_mask;    // Маска сканкода PS/2
}; //__packed

uint8_t ibuff[KEY_ARRAY_SIZE_RP2040];                // буфер для данных  из USB RP2040 to i2c

uint8_t i2c_dev[MAX_QNT_DEVICES];
uint8_t i2c_data[KEY_ARRAY_SIZE * MAX_QNT_DEVICES];
static uint8_t cnt_dev;
uint8_t val[2];
uint8_t scan;
uint8_t oldscan;

int state;
extern kb_u_state kb_st_ps2;
kbd_markup *layout;

void i2c_kbd_deinit();

int i2c_kbd_data_in();

bool i2c_scan_devices();

bool i2c_decode_kbd();

bool i2c_kbd_start();

#ifdef __cplusplus
}
#endif