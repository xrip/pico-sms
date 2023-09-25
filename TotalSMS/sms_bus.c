#include "assert.h"
#include "sms.h"
#include "sms_internal.h"
#include "sms_types.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern struct SMS_Core sms;

// this is mapped to any address space which is unmapped
// and *not* mirror, such as sega mapper writing to 0x0-0x8000 range
static uint8_t UNUSED_BANK[0x400];

static void sega_mapper_update_slot0()
{
    // if we have bios and is loaded, do not map rom here!
    if (SMS_has_bios() && !sms.memory_control.bios_rom_disable)
    {
        return;
    }

    const size_t offset = 0x4000 * sms.cart.mappers.sega.fffd;

    // this is fixed, never updated!
    sms.rmap[0x00] = sms.rom;

    for (size_t i = 1; i < 0x10; ++i)
    {
        // only the first 15 banks are saved
        sms.rmap[i] = sms.rom + offset + (0x400 * i);
    }
}

static void sega_mapper_update_slot1()
{
    const size_t offset = 0x4000 * sms.cart.mappers.sega.fffe;

    for (size_t i = 0; i < 0x10; ++i)
    {
        sms.rmap[i + 0x10] = sms.rom + offset + (0x400 * i);
    }
}

static void sega_mapper_update_slot2()
{
    const size_t offset = 0x4000 * sms.cart.mappers.sega.ffff;

    for (size_t i = 0; i < 0x10; ++i)
    {
        sms.rmap[i + 0x20] = sms.rom + offset + (0x400 * i);
        sms.wmap[i + 0x20] = UNUSED_BANK;
    }
}

static void sega_mapper_update_ram0()
{
    sms.cart.sram_used = true;

    for (size_t i = 0; i < 0x10; ++i)
    {
        sms.rmap[i + 0x20] = sms.cart.ram[sms.cart.mappers.sega.fffc.ram_bank_select] + (0x400 * i);
        sms.wmap[i + 0x20] = sms.cart.ram[sms.cart.mappers.sega.fffc.ram_bank_select] + (0x400 * i);
    }
}

static void setup_mapper_unused_ram()
{
    for (int i = 0; i < 0x40; i++)
    {
        sms.rmap[i] = UNUSED_BANK;
        sms.wmap[i] = UNUSED_BANK;
    }
}

static void setup_mapper_none()
{
    for (int i = 0; i < 0x30; i++)
    {
        sms.rmap[i] = sms.rom + (0x400 * (i % sms.rom_mask));
    }

    // sg only has 1k of ram
    // todo: maybe have different variants of MAPPER_NONE for each system?
    // such as sms, sg, msx etc
    const uint8_t ram_mask = SMS_is_system_type_sg() ? 0x0 : 0x7;

    for (int i = 0; i < 0x10; ++i)
    {
        sms.rmap[0x30 + i] = sms.system_ram + (0x400 * (i & ram_mask));
        sms.wmap[0x30 + i] = sms.system_ram + (0x400 * (i & ram_mask));
    }
}

static void init_mapper_sega()
{
    // control is reset to zero
    memset(&sms.cart.mappers.sega.fffc, 0, sizeof(sms.cart.mappers.sega.fffc));

    // default banks
    sms.cart.mappers.sega.fffd = 0;
    sms.cart.mappers.sega.fffe = 1;
    sms.cart.mappers.sega.ffff = 2;
}

static void setup_mapper_sega()
{
    for (int i = 0; i < 0x10; ++i)
    {
        sms.rmap[0x30 + i] = sms.system_ram + (0x400 * (i & 0x7));
        sms.wmap[0x30 + i] = sms.system_ram + (0x400 * (i & 0x7));
    }

    sega_mapper_update_slot0();
    sega_mapper_update_slot1();
    sega_mapper_update_slot2();
}

static void codemaster_mapper_update_slot(const uint8_t slot, const uint8_t value)
{
    sms.cart.mappers.codemasters.slot[slot] = value & sms.cart.max_bank_mask;
    const size_t offset = 0x4000 * sms.cart.mappers.codemasters.slot[slot];

    for (size_t i = 0; i < 0x10; ++i)
    {
        sms.rmap[i + (0x10 * slot)] = sms.rom + offset + (0x400 * i);
    }
}

static void codemaster_mapper_update_slot0(const uint8_t value)
{
    codemaster_mapper_update_slot(0, value);
}

static void codemaster_mapper_update_slot1(const uint8_t value)
{
    // for codemaster games that feature on-cart ram
    // writing to ctrl-1 with bit7 set maps ram 0xA000-0xC000
    if (IS_BIT_SET(value, 7))
    {
        for (size_t i = 0; i < 0x8; ++i)
        {
            sms.rmap[i + 0x28] = sms.cart.ram[0] + (0x400 * i);
            sms.wmap[i + 0x28] = sms.cart.ram[0] + (0x400 * i);
        }

        sms.cart.mappers.codemasters.ram_mapped = true;
        return;
    }
        // was ram previously mapped?
    else if (sms.cart.mappers.codemasters.ram_mapped)
    {
        sms.cart.mappers.codemasters.ram_mapped = false;

        // unmap ram and re-map the rom
        for (size_t i = 0; i < 0x8; ++i)
        {
            sms.wmap[i + 0x28] = UNUSED_BANK;
        }

        codemaster_mapper_update_slot(2, sms.cart.mappers.codemasters.slot[2]);
    }

    codemaster_mapper_update_slot(1, value);
}

static void codemaster_mapper_update_slot2(const uint8_t value)
{
    // NOTE: if ram is mapped (ernie elfs golf), then does writes to this
    // reg remap rom? or are they ignored?
    if (sms.cart.mappers.codemasters.ram_mapped)
    {
        sms.cart.mappers.codemasters.slot[2] = value & sms.cart.max_bank_mask;
        const size_t offset = 0x4000 * sms.cart.mappers.codemasters.slot[2];

        for (size_t i = 0; i < 0x8; ++i)
        {
            sms.rmap[i + 0x20] = sms.rom + offset + (0x400 * i);
        }
    }
    else
    {
        codemaster_mapper_update_slot(2, value);
    }
}

static void init_mapper_codemaster()
{
    sms.cart.mappers.codemasters.slot[0] = 0;
    sms.cart.mappers.codemasters.slot[1] = 1;
    sms.cart.mappers.codemasters.slot[2] = 2;
    sms.cart.mappers.codemasters.ram_mapped = false;
}

static void setup_mapper_codemaster()
{
    for (int i = 0; i < 0x10; ++i)
    {
        sms.rmap[0x30 + i] = sms.system_ram + (0x400 * (i & 0x7));
        sms.wmap[0x30 + i] = sms.system_ram + (0x400 * (i & 0x7));
    }

    codemaster_mapper_update_slot0(sms.cart.mappers.codemasters.slot[0]);
    codemaster_mapper_update_slot1(sms.cart.mappers.codemasters.slot[1]);
    codemaster_mapper_update_slot2(sms.cart.mappers.codemasters.slot[2]);
}

static void setup_mapper_dahjee_a()
{
    for (int i = 0; i < 0x30; i++)
    {
        sms.rmap[i] = sms.rom + (0x400 * (i % sms.rom_mask));
    }

    // has 8K mapped at 0x2000-0x3FFF
    for (int i = 0; i < 0x8; i++)
    {
        sms.rmap[0x8 + i] = sms.cart.ram[0] + (0x400 * i);
        sms.wmap[0x8 + i] = sms.cart.ram[0] + (0x400 * i);
    }

    // normal 1K mapping 0xC000-0xFFFF
    for (int i = 0; i < 0x10; ++i)
    {
        sms.rmap[0x30 + i] = sms.system_ram;
        sms.wmap[0x30 + i] = sms.system_ram;
    }
}

static void setup_mapper_dahjee_b()
{
    for (int i = 0; i < 0x30; i++)
    {
        sms.rmap[i] = sms.rom + (0x400 * (i % sms.rom_mask));
    }

    // 8K ram mapped 0xC000-0xFFFF
    for (int i = 0; i < 0x10; i++)
    {
        sms.rmap[0x30 + i] = sms.cart.ram[0] + (0x400 * (i&7));
        sms.wmap[0x30 + i] = sms.cart.ram[0] + (0x400 * (i&7));
    }
}

static void setup_mapper_castle()
{
    for (int i = 0; i < 0x20; i++)
    {
        sms.rmap[i] = sms.rom + (0x400 * (i % sms.rom_mask));
    }

    // 8k ram mapping 0x8000-0xBFFF
    for (int i = 0; i < 0x10; i++)
    {
        sms.rmap[0x20 + i] = sms.cart.ram[0] + (0x400 * (i&7));
        sms.wmap[0x20 + i] = sms.cart.ram[0] + (0x400 * (i&7));
    }

    // normal 1k mapping 0xC000-0xFFFF
    for (int i = 0; i < 0x10; ++i)
    {
        sms.rmap[0x30 + i] = sms.system_ram;
        sms.wmap[0x30 + i] = sms.system_ram;
    }
}

static void setup_mapper_othello()
{
    for (int i = 0; i < 0x20; i++)
    {
        sms.rmap[i] = sms.rom + (0x400 * (i % sms.rom_mask));
    }

    // 2K ram mapped 0x8000-0xFFFF
    for (int i = 0; i < 0x20; i++)
    {
        sms.rmap[0x20 + i] = sms.cart.ram[0] + (0x400 * (i&1));
        sms.wmap[0x20 + i] = sms.cart.ram[0] + (0x400 * (i&1));
    }
}

void mapper_update()
{
    // sets all banks to point to unused_bank[0x400]
    setup_mapper_unused_ram();

    switch (sms.cart.mapper_type)
    {
        case MAPPER_TYPE_SEGA:
            setup_mapper_sega();
            break;

        case MAPPER_TYPE_CODEMASTERS:
            setup_mapper_codemaster();
            break;

        case MAPPER_TYPE_NONE:
            setup_mapper_none();
            break;

        case MAPPER_TYPE_DAHJEE_A:
            setup_mapper_dahjee_a();
            break;

        case MAPPER_TYPE_DAHJEE_B:
            setup_mapper_dahjee_b();
            break;

        case MAPPER_TYPE_THE_CASTLE:
            setup_mapper_castle();
            break;

        case MAPPER_TYPE_OTHELLO:
            setup_mapper_othello();
            break;
    }

    // map the bios is enabled (if we have it)
    if (!sms.memory_control.bios_rom_disable)
    {
        assert(SMS_has_bios() && "bios was mapped, but we dont have it!");

        if (SMS_has_bios())
        {
            // usually 8 (8kib)
            const size_t map_max = sms.bios_size / 0x400;

            for (size_t i = 0; i < map_max; ++i)
            {
                sms.rmap[i] = sms.bios + (0x400 * i);
            }

            // not really needed, but just in case
            for (size_t i = map_max; i < 0x10; ++i)
            {
                sms.rmap[i] = sms.rom + (0x400 * i);
            }
        }
    }
}

void mapper_init()
{
    switch (sms.cart.mapper_type)
    {
        case MAPPER_TYPE_SEGA:
            init_mapper_sega();
            break;

        case MAPPER_TYPE_CODEMASTERS:
            init_mapper_codemaster();
            break;

        case MAPPER_TYPE_NONE:
        case MAPPER_TYPE_DAHJEE_A:
        case MAPPER_TYPE_DAHJEE_B:
        case MAPPER_TYPE_THE_CASTLE:
        case MAPPER_TYPE_OTHELLO:
            break;
    }

    mapper_update();
}

static void sega_mapper_fffx_write(const uint16_t addr, const uint8_t value)
{
    assert(sms.cart.mapper_type == MAPPER_TYPE_SEGA && "wrong mapper, how did we get here?!?");

    switch (addr)
    {
        case 0xFFFC: // Cartridge RAM mapper control
            // TODO: mapping at 0xC000
            sms.cart.mappers.sega.fffc.rom_write_enable = IS_BIT_SET(value, 7);
            sms.cart.mappers.sega.fffc.ram_enable_c0000 = IS_BIT_SET(value, 4);
            sms.cart.mappers.sega.fffc.ram_enable_80000 = IS_BIT_SET(value, 3);
            sms.cart.mappers.sega.fffc.ram_bank_select = IS_BIT_SET(value, 2);
            sms.cart.mappers.sega.fffc.bank_shift = value & 0x3;

            // assert(!sms.cart.mappers.sega.fffc.rom_write_enable && "rom write enable!");
            assert(!sms.cart.mappers.sega.fffc.ram_enable_c0000 && "unimp ram_enable_c0000");
            assert(!sms.cart.mappers.sega.fffc.bank_shift && "unimp bank_shift");

            if (sms.cart.mappers.sega.fffc.ram_enable_80000)
            {
                sega_mapper_update_ram0();
                SMS_log("game is mapping sram to rom region 0x8000!");
            }
            else
            {
                // unamp ram
                sega_mapper_update_slot2();
            }
            break;

        case 0xFFFD: // Mapper slot 0 control
            sms.cart.mappers.sega.fffd = value & sms.cart.max_bank_mask;
            sega_mapper_update_slot0();
            break;

        case 0xFFFE: // Mapper slot 1 control
            sms.cart.mappers.sega.fffe = value & sms.cart.max_bank_mask;
            sega_mapper_update_slot1();
            break;

        case 0xFFFF: // Mapper slot 2 control
            sms.cart.mappers.sega.ffff = value & sms.cart.max_bank_mask;
            if (sms.cart.mappers.sega.fffc.ram_enable_80000 == false)
            {
                sega_mapper_update_slot2();
            }
            break;
    }
}

uint8_t FORCE_INLINE SMS_read8(const uint16_t addr)
{
    assert(sms.rmap[addr >> 10] && "NULL ptr in rmap!");
    return sms.rmap[addr >> 10][addr & 0x3FF];
}

void FORCE_INLINE SMS_write8(const uint16_t addr, const uint8_t value)
{
    // specific mapper writes
    switch (sms.cart.mapper_type)
    {
        case MAPPER_TYPE_SEGA:
            if (addr >= 0xFFFC)
            {
                sega_mapper_fffx_write(addr, value);
            }
            break;

            // control regs are not mirrored to ram (written values cannot be read back)
        case MAPPER_TYPE_CODEMASTERS:
            if (addr <= 0x3FFF)
            {
                assert(!(addr & 0xFFF) && "codemaster ctrl mirror used!");
                codemaster_mapper_update_slot0(value);
                return;
            }
            else if (addr >= 0x4000 && addr <= 0x7FFF)
            {
                assert(!(addr & 0xFFF) && "codemaster ctrl mirror used!");
                codemaster_mapper_update_slot1(value);
                return;
            }
            else if (addr >= 0x8000 && addr <= 0xBFFF)
            {
                if (!sms.cart.mappers.codemasters.ram_mapped || addr <= 0x9FFF)
                {
                    assert(!(addr & 0xFFF) && "codemaster ctrl mirror used!");
                    codemaster_mapper_update_slot2(value);
                    return;
                }
            }
            break;

        case MAPPER_TYPE_NONE:
        case MAPPER_TYPE_DAHJEE_A:
        case MAPPER_TYPE_DAHJEE_B:
        case MAPPER_TYPE_THE_CASTLE:
        case MAPPER_TYPE_OTHELLO:
            break;
    }

    assert(sms.wmap[addr >> 10] && "NULL ptr in wmap!");
    sms.wmap[addr >> 10][addr & 0x3FF] = value;
}

uint16_t FORCE_INLINE SMS_read16(const uint16_t addr)
{
    const uint16_t lo = SMS_read8(addr + 0);
    const uint16_t hi = SMS_read8(addr + 1);

    return (hi << 8) | lo;
}

void FORCE_INLINE SMS_write16(const uint16_t addr, const uint16_t value)
{
    SMS_write8(addr + 0, value & 0xFF);
    SMS_write8(addr + 1, value >> 8);
}

// https://www.smspower.org/Development/Port3E
static void IO_memory_control_write(const uint8_t value)
{
    const struct SMS_MemoryControlRegister old = sms.memory_control;

    sms.memory_control.exp_slot_disable  = IS_BIT_SET(value, 7);
    sms.memory_control.cart_slot_disable = IS_BIT_SET(value, 6);
    sms.memory_control.card_slot_disable = IS_BIT_SET(value, 5);
    sms.memory_control.work_ram_disable  = IS_BIT_SET(value, 4);
    sms.memory_control.bios_rom_disable  = IS_BIT_SET(value, 3);
    sms.memory_control.io_chip_disable   = IS_BIT_SET(value, 2);

    // bios either unmapped itself (likely) or got re-mapped (impossible)
    if (SMS_has_bios() && old.bios_rom_disable != sms.memory_control.bios_rom_disable)
    {
        mapper_update();
    }

    SMS_log("[memory_control]");
    SMS_log("\tmemory_control.exp_slot_disable: %u", sms.memory_control.exp_slot_disable);
    SMS_log("\tmemory_control.cart_slot_disable: %u", sms.memory_control.cart_slot_disable);
    SMS_log("\tmemory_control.card_slot_disable: %u", sms.memory_control.card_slot_disable);
    SMS_log("\tmemory_control.work_ram_disable: %u", sms.memory_control.work_ram_disable);
    SMS_log("\tmemory_control.bios_rom_disable: %u", sms.memory_control.bios_rom_disable);
    SMS_log("\tmemory_control.io_chip_disable: %u", sms.memory_control.io_chip_disable);
    // assert(.memory_control.work_ram_disable == 0 && "wram disabled!");
}

// todo: PORT 0x3F
static void IO_control_write(const uint8_t value)
{
    /*
    bit 0: controller 1 button 2 direction (1=input 0=output)
    bit 1: controller 1 TH direction (1=input 0=output)
    bit 2: controller 2 button 2 direction (1=input 0=output)
    bit 3: controller 2 TH direction (1=input 0=output)

    bit 4: controller 1 button 2 output level (1=high 0=low)
    bit 5: controller 1 TH output level (1=high 0=low*)
    bit 6: controller 2 button 2 output level (1=high 0=low)
    bit 7: controller 2 TH output level (1=high 0=low*)
    */
    // 0, 2, 4, 5, 6, 7
    // 0, 2, 4, 6
    (void)sms; (void)value;
    SMS_log("IO_control_write: 0x%02X", value);
}

static FORCE_INLINE uint8_t IO_read_vcounter()
{
    return sms.vdp.vcount_port;
}

const uint8_t vdp_hcounter[228] = {
        0xE9,0xEA,0xEA,0xEB,0xEC,0xED,0xED,0xEE,0xEF,0xF0,0xF0,0xF1,0xF2,0xF3,0xF3,0xF4,
        0xF5,0xF6,0xF6,0xF7,0xF8,0xF9,0xF9,0xFA,0xFB,0xFC,0xFC,0xFD,0xFE,0xFF,0xFF,0x00,
        0x01,0x02,0x02,0x03,0x04,0x05,0x05,0x06,0x07,0x08,0x08,0x09,0x0A,0x0B,0x0B,0x0C,
        0x0D,0x0E,0x0E,0x0F,0x10,0x11,0x11,0x12,0x13,0x14,0x14,0x15,0x16,0x17,0x17,0x18,
        0x19,0x1A,0x1A,0x1B,0x1C,0x1D,0x1D,0x1E,0x1F,0x20,0x20,0x21,0x22,0x23,0x23,0x24,
        0x25,0x26,0x26,0x27,0x28,0x29,0x29,0x2A,0x2B,0x2C,0x2C,0x2D,0x2E,0x2F,0x2F,0x30,
        0x31,0x32,0x32,0x33,0x34,0x35,0x35,0x36,0x37,0x38,0x38,0x39,0x3A,0x3B,0x3B,0x3C,
        0x3D,0x3E,0x3E,0x3F,0x40,0x41,0x41,0x42,0x43,0x44,0x44,0x45,0x46,0x47,0x47,0x48,
        0x49,0x4A,0x4A,0x4B,0x4C,0x4D,0x4D,0x4E,0x4F,0x50,0x50,0x51,0x52,0x53,0x53,0x54,
        0x55,0x56,0x56,0x57,0x58,0x59,0x59,0x5A,0x5B,0x5C,0x5C,0x5D,0x5E,0x5F,0x5F,0x60,
        0x61,0x62,0x62,0x63,0x64,0x65,0x65,0x66,0x67,0x68,0x68,0x69,0x6A,0x6B,0x6B,0x6C,
        0x6D,0x6E,0x6E,0x6F,0x70,0x71,0x71,0x72,0x73,0x74,0x74,0x75,0x76,0x77,0x77,0x78,
        0x79,0x7A,0x7A,0x7B,0x7C,0x7D,0x7D,0x7E,0x7F,0x80,0x80,0x81,0x82,0x83,0x83,0x84,
        0x85,0x86,0x86,0x87,0x88,0x89,0x89,0x8A,0x8B,0x8C,0x8C,0x8D,0x8E,0x8F,0x8F,0x90,
        0x91,0x92,0x92,0x93,
};

static FORCE_INLINE uint8_t IO_read_hcounter()
{
    // docs say that this is a 9-bit counter, but only upper 8-bits read
    return vdp_hcounter[sms.vdp.cycles % 228];
    //return (uint16_t)((float)sms.vdp.cycles * 1.5F) >> 1;
}

static FORCE_INLINE uint8_t IO_vdp_status_read()
{
    sms.vdp.control_latch = false;

    return vdp_status_flag_read();
}

static FORCE_INLINE uint8_t IO_vdp_data_read()
{
    sms.vdp.control_latch = false;

    const uint8_t data = sms.vdp.buffer_read_data;

    sms.vdp.buffer_read_data = sms.vdp.vram[sms.vdp.addr];
    sms.vdp.addr = (sms.vdp.addr + 1) & 0x3FFF;

    return data;
}

static void IO_vdp_cram_gg_write(const uint8_t value)
{
    // even addr stores byte to latch, odd writes 2 bytes
    if (sms.vdp.addr & 1)
    {
        const uint8_t rg_index = (sms.vdp.addr - 1) & 0x3F;
        const uint8_t b_index = sms.vdp.addr & 0x3F;

        // check is the colour has changed, if so, set dirty
        if (sms.vdp.cram[rg_index] != sms.vdp.cram_gg_latch || sms.vdp.cram[b_index] != value)
        {
            sms.vdp.dirty_cram_min = SMS_MIN(sms.vdp.dirty_cram_min, rg_index);
            sms.vdp.dirty_cram_max = SMS_MAX(sms.vdp.dirty_cram_max, rg_index+1);
            sms.vdp.dirty_cram[rg_index] = true;
        }

        sms.vdp.cram[rg_index] = sms.vdp.cram_gg_latch;
        sms.vdp.cram[b_index] = value;
    }
    else
    {
        // latches the r,g values
        sms.vdp.cram_gg_latch = value;
    }
}

static void IO_vdp_cram_sms_write(const uint8_t value)
{
    const uint8_t index = sms.vdp.addr & 0x1F;

    if (sms.vdp.cram[index] != value)
    {
        sms.vdp.cram[index] = value;
        sms.vdp.dirty_cram[index] = true;
        sms.vdp.dirty_cram_min = SMS_MIN(sms.vdp.dirty_cram_min, index);
        sms.vdp.dirty_cram_max = SMS_MAX(sms.vdp.dirty_cram_max, index+1);
    }
}

static void IO_vdp_data_write(const uint8_t value)
{
    sms.vdp.control_latch = false;
    // writes store the new value in the buffered_data
    sms.vdp.buffer_read_data = value;

    switch (sms.vdp.code)
    {
        case VDP_CODE_VRAM_WRITE_LOAD:
        case VDP_CODE_VRAM_WRITE:
        case VDP_CODE_REG_WRITE:
            // mark entry as dirty if modified
            sms.vdp.dirty_vram[sms.vdp.addr >> 2] |= sms.vdp.vram[sms.vdp.addr] != value;
            sms.vdp.vram[sms.vdp.addr] = value;
            sms.vdp.addr = (sms.vdp.addr + 1) & 0x3FFF;
            break;

        case VDP_CODE_CRAM_WRITE:
            switch (SMS_get_system_type())
            {
                case SMS_System_SMS:
                    IO_vdp_cram_sms_write(value);
                    break;

                case SMS_System_GG:
                    IO_vdp_cram_gg_write(value);
                    break;

                case SMS_System_SG1000:
                    assert(!"sg1000 cram write, check what should happen here!");
                    break;
            }

            sms.vdp.addr = (sms.vdp.addr + 1) & 0x3FFF;
            break;
    }
}

static void IO_vdp_control_write(const uint8_t value)
{
    if (sms.vdp.control_latch)
    {
        sms.vdp.control_word = (sms.vdp.control_word & 0xFF) | (value << 8);
        sms.vdp.code = (value >> 6) & 3;
        sms.vdp.control_latch = false;

        switch (sms.vdp.code)
        {
            // code0 immediatley loads a byte from vram into buffer
            case VDP_CODE_VRAM_WRITE_LOAD:
                sms.vdp.addr = sms.vdp.control_word & 0x3FFF;
                sms.vdp.buffer_read_data = sms.vdp.vram[sms.vdp.addr];
                sms.vdp.addr = (sms.vdp.addr + 1) & 0x3FFF;
                break;

            case VDP_CODE_VRAM_WRITE:
                sms.vdp.addr = sms.vdp.control_word & 0x3FFF;
                break;

            case VDP_CODE_REG_WRITE:
                vdp_io_write(value & 0xF, sms.vdp.control_word & 0xFF);
                break;

            case VDP_CODE_CRAM_WRITE:
                sms.vdp.addr = sms.vdp.control_word & 0x3FFF;
                break;
        }
    }
    else
    {
        sms.vdp.addr = (sms.vdp.addr & 0x3F00) | value;
        sms.vdp.control_word = value;
        sms.vdp.control_latch = true;
    }
}

static uint8_t IO_gamegear_read(const uint8_t addr)
{
    switch (addr & 0x7)
    {
        case 0x0: return sms.port.gg_regs[0x0] | 0x1F;
        case 0x1: return /* 0x7F; */ sms.port.gg_regs[0x1];
        case 0x2: return /* 0xFF; */ sms.port.gg_regs[0x2];
        case 0x3: return /* 0x00; */ sms.port.gg_regs[0x3];
        case 0x4: return /* 0xFF; */ sms.port.gg_regs[0x4];
        case 0x5: return /* 0x00; */ sms.port.gg_regs[0x5];
        default: __builtin_unreachable();
    }

    UNREACHABLE(0xFF);
}

static void IO_gamegear_write(const uint8_t addr, const uint8_t value)
{
    switch (addr & 0x7)
    {
        case 0x1: sms.port.gg_regs[0x1] = value; break;
        case 0x2: sms.port.gg_regs[0x2] = value; break;
        case 0x3: sms.port.gg_regs[0x3] = value; break;
        case 0x4: break;
        case 0x5: sms.port.gg_regs[0x5] = value; break;

        case 0x6: // sets stereo output for psg
            sms.psg.channel_enable[0][0] = IS_BIT_SET(value, 0);
            sms.psg.channel_enable[0][1] = IS_BIT_SET(value, 1);

            sms.psg.channel_enable[1][0] = IS_BIT_SET(value, 2);
            sms.psg.channel_enable[1][1] = IS_BIT_SET(value, 3);

            sms.psg.channel_enable[2][0] = IS_BIT_SET(value, 4);
            sms.psg.channel_enable[2][1] = IS_BIT_SET(value, 5);

            sms.psg.channel_enable[3][0] = IS_BIT_SET(value, 6);
            sms.psg.channel_enable[3][1] = IS_BIT_SET(value, 7);
            break;
    }
}

uint8_t SMS_read_io(const uint8_t addr)
{
    switch (addr)
    {
        case 0x00: case 0x01: case 0x02: case 0x03:
        case 0x04: case 0x05:
            if (SMS_is_system_type_gg())
            {
                return IO_gamegear_read(addr);
            }
            else
            {
                assert(!"reading from gg port in non gg mode, what should happen here?");
                return 0xFF;
            }

        case 0x06: case 0x07:
        case 0x08: case 0x09: case 0x0A: case 0x0B:
        case 0x0C: case 0x0D: case 0x0E: case 0x0F:
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x19: case 0x1A: case 0x1B:
        case 0x1C: case 0x1D: case 0x1E: case 0x1F:
        case 0x20: case 0x21: case 0x22: case 0x23:
        case 0x24: case 0x25: case 0x26: case 0x27:
        case 0x28: case 0x29: case 0x2A: case 0x2B:
        case 0x2C: case 0x2D: case 0x2E: case 0x2F:
        case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x38: case 0x39: case 0x3A: case 0x3B:
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
            // note: ristar (GG) reads from port $22 for some reason...
            SMS_log("[PORT-READ] 0x%02X last byte of the instruction", addr);
            return 0xFF; // todo:

        case 0x40: case 0x42: case 0x44: case 0x46:
        case 0x48: case 0x4A: case 0x4C: case 0x4E:
        case 0x50: case 0x52: case 0x54: case 0x56:
        case 0x58: case 0x5A: case 0x5C: case 0x5E:
        case 0x60: case 0x62: case 0x64: case 0x66:
        case 0x68: case 0x6A: case 0x6C: case 0x6E:
        case 0x70: case 0x72: case 0x74: case 0x76:
        case 0x78: case 0x7A: case 0x7C: case 0x7E:
            return IO_read_vcounter();

        case 0x41: case 0x43: case 0x45: case 0x47:
        case 0x49: case 0x4B: case 0x4D: case 0x4F:
        case 0x51: case 0x53: case 0x55: case 0x57:
        case 0x59: case 0x5B: case 0x5D: case 0x5F:
        case 0x61: case 0x63: case 0x65: case 0x67:
        case 0x69: case 0x6B: case 0x6D: case 0x6F:
        case 0x71: case 0x73: case 0x75: case 0x77:
        case 0x79: case 0x7B: case 0x7D: case 0x7F:
            return IO_read_hcounter();

        case 0x80: case 0x82: case 0x84: case 0x86:
        case 0x88: case 0x8A: case 0x8C: case 0x8E:
        case 0x90: case 0x92: case 0x94: case 0x96:
        case 0x98: case 0x9A: case 0x9C: case 0x9E:
        case 0xA0: case 0xA2: case 0xA4: case 0xA6:
        case 0xA8: case 0xAA: case 0xAC: case 0xAE:
        case 0xB0: case 0xB2: case 0xB4: case 0xB6:
        case 0xB8: case 0xBA: case 0xBC: case 0xBE:
            return IO_vdp_data_read();

        case 0x81: case 0x83: case 0x85: case 0x87:
        case 0x89: case 0x8B: case 0x8D: case 0x8F:
        case 0x91: case 0x93: case 0x95: case 0x97:
        case 0x99: case 0x9B: case 0x9D: case 0x9F:
        case 0xA1: case 0xA3: case 0xA5: case 0xA7:
        case 0xA9: case 0xAB: case 0xAD: case 0xAF:
        case 0xB1: case 0xB3: case 0xB5: case 0xB7:
        case 0xB9: case 0xBB: case 0xBD: case 0xBF:
            return IO_vdp_status_read();

        case 0xC0: case 0xC2: case 0xC4: case 0xC6:
        case 0xC8: case 0xCA: case 0xCC: case 0xCE:
        case 0xD0: case 0xD2: case 0xD4: case 0xD6:
        case 0xD8: case 0xDA: case 0xDC: case 0xDE:
        case 0xE0: case 0xE2: case 0xE4: case 0xE6:
        case 0xE8: case 0xEA: case 0xEC: case 0xEE:
        case 0xF0: case 0xF2: case 0xF4: case 0xF6:
        case 0xF8: case 0xFA: case 0xFC: case 0xFE:
            return sms.port.a;

        case 0xC1: case 0xC3: case 0xC5: case 0xC7:
        case 0xC9: case 0xCB: case 0xCD: case 0xCF:
        case 0xD1: case 0xD3: case 0xD5: case 0xD7:
        case 0xD9: case 0xDB: case 0xDD: case 0xDF:
        case 0xE1: case 0xE3: case 0xE5: case 0xE7:
        case 0xE9: case 0xEB: case 0xED: case 0xEF:
        case 0xF1: case 0xF3: case 0xF5: case 0xF7:
        case 0xF9: case 0xFB: case 0xFD: case 0xFF:
            return sms.port.b;
        default: __builtin_unreachable();
    }

    UNREACHABLE(0xFF);
}

void SMS_write_io(const uint8_t addr, const uint8_t value)
{
    switch (addr)
    {
        // GG regs
        case 0x00: case 0x01: case 0x02: case 0x03:
        case 0x04: case 0x05: case 0x06:
            if (SMS_is_system_type_gg())
            {
                IO_gamegear_write(addr, value);
            }
            else
            {
                if (addr & 1) // odd / even split
                {
                    IO_control_write(value);
                }
                else
                {
                    IO_memory_control_write(value);
                }
            }
            break;

        case 0x08: case 0x0A: case 0x0C: case 0x0E:
        case 0x10: case 0x12: case 0x14: case 0x16:
        case 0x18: case 0x1A: case 0x1C: case 0x1E:
        case 0x20: case 0x22: case 0x24: case 0x26:
        case 0x28: case 0x2A: case 0x2C: case 0x2E:
        case 0x30: case 0x32: case 0x34: case 0x36:
        case 0x38: case 0x3A: case 0x3C: case 0x3E:
            IO_memory_control_write(value);
            break;

        case 0x07:
        case 0x09: case 0x0B: case 0x0D: case 0x0F:
        case 0x11: case 0x13: case 0x15: case 0x17:
        case 0x19: case 0x1B: case 0x1D: case 0x1F:
        case 0x21: case 0x23: case 0x25: case 0x27:
        case 0x29: case 0x2B: case 0x2D: case 0x2F:
        case 0x31: case 0x33: case 0x35: case 0x37:
        case 0x39: case 0x3B: case 0x3D: case 0x3F:
            IO_control_write(value);
            break;

        case 0x40: case 0x41: case 0x42: case 0x43:
        case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x48: case 0x49: case 0x4A: case 0x4B:
        case 0x4C: case 0x4D: case 0x4E: case 0x4F:
        case 0x50: case 0x51: case 0x52: case 0x53:
        case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x58: case 0x59: case 0x5A: case 0x5B:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        case 0x60: case 0x61: case 0x62: case 0x63:
        case 0x64: case 0x65: case 0x66: case 0x67:
        case 0x68: case 0x69: case 0x6A: case 0x6B:
        case 0x6C: case 0x6D: case 0x6E: case 0x6F:
        case 0x70: case 0x71: case 0x72: case 0x73:
        case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7A: case 0x7B:
        case 0x7C: case 0x7D: case 0x7E: case 0x7F:
            psg_reg_write(value);
            break;

        case 0x80: case 0x82: case 0x84: case 0x86:
        case 0x88: case 0x8A: case 0x8C: case 0x8E:
        case 0x90: case 0x92: case 0x94: case 0x96:
        case 0x98: case 0x9A: case 0x9C: case 0x9E:
        case 0xA0: case 0xA2: case 0xA4: case 0xA6:
        case 0xA8: case 0xAA: case 0xAC: case 0xAE:
        case 0xB0: case 0xB2: case 0xB4: case 0xB6:
        case 0xB8: case 0xBA: case 0xBC: case 0xBE:
            IO_vdp_data_write(value);
            break;

        case 0x81: case 0x83: case 0x85: case 0x87:
        case 0x89: case 0x8B: case 0x8D: case 0x8F:
        case 0x91: case 0x93: case 0x95: case 0x97:
        case 0x99: case 0x9B: case 0x9D: case 0x9F:
        case 0xA1: case 0xA3: case 0xA5: case 0xA7:
        case 0xA9: case 0xAB: case 0xAD: case 0xAF:
        case 0xB1: case 0xB3: case 0xB5: case 0xB7:
        case 0xB9: case 0xBB: case 0xBD: case 0xBF:
            IO_vdp_control_write(value);
            break;
    }
}
