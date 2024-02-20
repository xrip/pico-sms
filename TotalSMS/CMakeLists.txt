add_library(TotalSMS INTERFACE)

target_sources(TotalSMS
INTERFACE
    sms_bus.c
    sms_z80.c
    sms_joypad.c
    sms.c
    sms_psg.c
    sms_vdp.c
    sms_rom_database.c
)
