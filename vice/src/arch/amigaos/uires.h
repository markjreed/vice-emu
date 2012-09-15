/*
 * uires.h
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef VICE_UIRES_H_
#define VICE_UIRES_H_

enum {
    IDM_NONE = 0,
    IDM_AUTOSTART, IDM_ATTACH_8, IDM_ATTACH_9, IDM_ATTACH_10, IDM_ATTACH_11,
    IDM_DETACH_8, IDM_DETACH_9, IDM_DETACH_10, IDM_DETACH_11, IDM_DETACH_ALL,
    IDM_FLIP_ADD, IDM_FLIP_REMOVE, IDM_FLIP_NEXT, IDM_FLIP_PREVIOUS,
    IDM_FLIP_LOAD, IDM_FLIP_SAVE, IDM_ATTACH_TAPE, IDM_DETACH_TAPE,
    IDM_DATASETTE_CONTROL_STOP, IDM_DATASETTE_CONTROL_START,
    IDM_DATASETTE_CONTROL_FORWARD, IDM_DATASETTE_CONTROL_REWIND,
    IDM_DATASETTE_CONTROL_RECORD, IDM_DATASETTE_CONTROL_RESET,
    IDM_DATASETTE_RESET_COUNTER, IDM_CART_ATTACH_CRT, IDM_CART_ATTACH_8KB,
    IDM_CART_ATTACH_16KB, IDM_CART_ATTACH_AR, IDM_CART_ATTACH_AR3,
    IDM_CART_ATTACH_AR4, IDM_CART_ATTACH_STARDOS, IDM_CART_ATTACH_AT,
    IDM_CART_ATTACH_EPYX, IDM_CART_ATTACH_IEEE488, IDM_CART_ATTACH_RR,
    IDM_CART_ATTACH_IDE64, IDM_CART_ATTACH_SS4, IDM_CART_ATTACH_SS5,
    IDM_CART_ATTACH_STB, IDM_CART_SET_DEFAULT, IDM_TOGGLE_CART_RESET,
    IDM_LOAD_CART_1XXX, IDM_UNLOAD_CART_1XXX, IDM_LOAD_CART_2_3XXX,
    IDM_UNLOAD_CART_2_3XXX, IDM_LOAD_CART_4_5XXX, IDM_UNLOAD_CART_4_5XXX,
    IDM_LOAD_CART_6_7XXX, IDM_UNLOAD_CART_6_7XXX, IDM_CART_VIC20_GENERIC,
    IDM_CART_VIC20_FP, IDM_CART_VIC20_MEGACART, IDM_CART_VIC20_FINAL_EXPANSION,
    IDM_CART_VIC20_SMART_ATTACH, IDM_CART_VIC20_8KB_2000,
    IDM_CART_VIC20_16KB_4000, IDM_CART_VIC20_8KB_6000, IDM_CART_VIC20_8KB_A000,
    IDM_CART_VIC20_4KB_B000, IDM_CART_ATTACH_C0LO, IDM_CART_ATTACH_C0HI,
    IDM_CART_ATTACH_C1LO, IDM_CART_ATTACH_C1HI, IDM_CART_ATTACH_C2LO,
    IDM_CART_ATTACH_C2HI, IDM_CART_DETACH, IDM_CART_FREEZE, IDM_PAUSE,
    IDM_MONITOR, IDM_RESET_HARD, IDM_RESET_SOFT, IDM_RESET_DRIVE8,
    IDM_RESET_DRIVE9, IDM_RESET_DRIVE10, IDM_RESET_DRIVE11, IDM_EXIT, IDM_COPY,
    IDM_PASTE, IDM_SNAPSHOT_LOAD, IDM_SNAPSHOT_SAVE, IDM_EVENT_TOGGLE_RECORD,
    IDM_EVENT_TOGGLE_PLAYBACK, IDM_EVENT_SETMILESTONE,
    IDM_EVENT_RESETMILESTONE, IDM_EVENT_START_MODE_SAVE,
    IDM_EVENT_START_MODE_LOAD, IDM_EVENT_START_MODE_RESET,
    IDM_EVENT_START_MODE_PLAYBACK, IDM_EVENT_DIRECTORY, IDM_MEDIAFILE,
    IDM_SOUND_RECORD_START, IDM_SOUND_RECORD_STOP, IDM_REFRESH_RATE_AUTO,
    IDM_REFRESH_RATE_1, IDM_REFRESH_RATE_2, IDM_REFRESH_RATE_3,
    IDM_REFRESH_RATE_4, IDM_REFRESH_RATE_5, IDM_REFRESH_RATE_6,
    IDM_REFRESH_RATE_7, IDM_REFRESH_RATE_8, IDM_REFRESH_RATE_9,
    IDM_REFRESH_RATE_10, IDM_MAXIMUM_SPEED_200, IDM_MAXIMUM_SPEED_100,
    IDM_MAXIMUM_SPEED_50, IDM_MAXIMUM_SPEED_20, IDM_MAXIMUM_SPEED_10,
    IDM_MAXIMUM_SPEED_NO_LIMIT, IDM_MAXIMUM_SPEED_CUSTOM, IDM_TOGGLE_WARP_MODE,
    IDM_TOGGLE_FULLSCREEN, IDM_TOGGLE_VIDEOCACHE, IDM_TOGGLE_DOUBLESIZE,
    IDM_TOGGLE_DOUBLESCAN, IDM_TOGGLE_VERTICAL_STRETCH,
    IDM_TOGGLE_VDC_DOUBLESIZE, IDM_TOGGLE_VDC_DOUBLESCAN, IDM_TOGGLE_VDC64KB,
    IDM_VDC_REV_0, IDM_VDC_REV_1, IDM_VDC_REV_2, IDM_TOGGLE_STATUSBAR,
    IDM_JOYKEYS_TOGGLE, IDM_OPPOSITE_JOY_DIR, IDM_TOGGLE_OVERLAY,
    IDM_SWAP_JOYSTICK, IDM_SWAP_USERPORT_JOYSTICK, IDM_TOGGLE_SOUND,
    IDM_IEEE488, IDM_TOGGLE_DRIVE_TRUE_EMULATION, IDM_TOGGLE_DRIVE_SOUND,
    IDM_TOGGLE_AUTOSTART_HANDLE_TDE, IDM_TOGGLE_VIRTUAL_DEVICES,
    IDM_SYNC_FACTOR_PAL, IDM_SYNC_FACTOR_NTSC, IDM_SYNC_FACTOR_NTSCOLD,
    IDM_MOUSE, IDM_PS2_MOUSE, IDM_AUTOSTART_SETTINGS, IDM_C64_MODEL_C64_PAL,
    IDM_C64_MODEL_C64C_PAL, IDM_C64_MODEL_C64_OLD_PAL, IDM_C64_MODEL_C64_NTSC,
    IDM_C64_MODEL_C64C_NTSC, IDM_C64_MODEL_C64_OLD_NTSC, IDM_C64_MODEL_DREAN,
    IDM_C64_MODEL_CUSTOM, IDM_PLUS4_MODEL_C16_PAL, IDM_PLUS4_MODEL_C16_NTSC,
    IDM_PLUS4_MODEL_PLUS4_PAL, IDM_PLUS4_MODEL_PLUS4_NTSC,
    IDM_PLUS4_MODEL_V364_NTSC, IDM_PLUS4_MODEL_C232_NTSC,
    IDM_C64DTV_MODEL_V2_PAL, IDM_C64DTV_MODEL_V2_NTSC, IDM_C64DTV_MODEL_V3_PAL,
    IDM_C64DTV_MODEL_V3_NTSC, IDM_C64DTV_MODEL_HUMMER_NTSC,
    IDM_C128_MODEL_C128_PAL, IDM_C128_MODEL_C128DCR_PAL,
    IDM_C128_MODEL_C128_NTSC, IDM_C128_MODEL_C128DCR_NTSC, IDM_PET_MODEL_2001,
    IDM_PET_MODEL_3008, IDM_PET_MODEL_3016, IDM_PET_MODEL_3032,
    IDM_PET_MODEL_3032B, IDM_PET_MODEL_4016, IDM_PET_MODEL_4032,
    IDM_PET_MODEL_4032B, IDM_PET_MODEL_8032, IDM_PET_MODEL_8096,
    IDM_PET_MODEL_8296, IDM_PET_MODEL_SUPERPET, IDM_CBM2_MODEL_610_PAL,
    IDM_CBM2_MODEL_610_NTSC, IDM_CBM2_MODEL_620_PAL, IDM_CBM2_MODEL_620_NTSC,
    IDM_CBM2_MODEL_620PLUS_PAL, IDM_CBM2_MODEL_620PLUS_NTSC,
    IDM_CBM2_MODEL_710_NTSC, IDM_CBM2_MODEL_720_NTSC,
    IDM_CBM2_MODEL_720PLUS_NTSC, IDM_CBM2_MODEL_510_PAL,
    IDM_CBM2_MODEL_510_NTSC, IDM_DEVICEMANAGER, IDM_TOGGLE_AUDIO_LEAK,
    IDM_PALETTE_SETTINGS, IDM_COLOR_SETTINGS, IDM_RENDER_FILTER,
    IDM_CRT_EMULATION_SETTINGS, IDM_TOGGLE_AUDIO_LEAK_2,
    IDM_PALETTE_SETTINGS_2, IDM_COLOR_SETTINGS_2, IDM_RENDER_FILTER_2,
    IDM_CRT_EMULATION_SETTINGS_2, IDM_DRIVE_SETTINGS, IDM_PRINTER_SETTINGS,
    IDM_DATASETTE_SETTINGS, IDM_VICII_SETTINGS, IDM_VIC_SETTINGS,
    IDM_TED_SETTINGS, IDM_JOY_SETTINGS, IDM_JOY_DEVICE_SELECTION,
    IDM_JOY_FIRE_SELECTION, IDM_KEYBOARD_SETTINGS, IDM_MOUSE_SETTINGS,
    IDM_SOUND_SETTINGS, IDM_SID_SETTINGS, IDM_CIA_SETTINGS,
    IDM_COMPUTER_ROM_SETTINGS, IDM_DRIVE_ROM_SETTINGS, IDM_RAM_SETTINGS,
    IDM_RS232_SETTINGS, IDM_REU_SETTINGS, IDM_GEORAM_SETTINGS,
    IDM_RAMCART_SETTINGS, IDM_DQBB_SETTINGS, IDM_ISEPIC_SETTINGS,
    IDM_PLUS60K_SETTINGS, IDM_PLUS256K_SETTINGS, IDM_EASYFLASH_SETTINGS,
    IDM_C64_256K_SETTINGS, IDM_MAGIC_VOICE_SETTINGS, IDM_EXPERT_SETTINGS,
    IDM_IDE64_SETTINGS, IDM_C128_SETTINGS, IDM_VIC20_SETTINGS,
    IDM_CBM2_SETTINGS, IDM_CBM5X0_SETTINGS, IDM_PET_SETTINGS,
    IDM_PLUS4_SETTINGS, IDM_TFE_SETTINGS, IDM_ACIA_SETTINGS,
    IDM_RS232USER_SETTINGS, IDM_NETWORK_SETTINGS, IDM_PETREU_SETTINGS,
    IDM_MMC64_SETTINGS, IDM_MMCREPLAY_SETTINGS, IDM_RETROREPLAY_SETTINGS,
    IDM_V364_SPEECH_SETTINGS, IDM_SIDCART_SETTINGS, IDM_PETDWW_SETTINGS,
    IDM_FINAL_EXPANSION_WRITEBACK, IDM_FP_WRITEBACK, IDM_MEGACART_WRITEBACK,
    IDM_MEGACART_WRITEBACK_FILE, IDM_DIGIMAX_SETTINGS, IDM_DS12C887RTC_SETTINGS,
    IDM_C64DTV_SETTINGS,
    IDM_SFX_SE_SETTINGS, IDM_SFX_SS_SETTINGS, IDM_TOGGLE_SFX_SS, IDM_BURST_MOD,
    IDM_SETTINGS_SAVE_FILE, IDM_SETTINGS_LOAD_FILE, IDM_SETTINGS_SAVE,
    IDM_SETTINGS_LOAD, IDM_SETTINGS_DEFAULT, IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT,
    IDM_TOGGLE_CONFIRM_ON_EXIT, IDM_LANGUAGE_ENGLISH, IDM_LANGUAGE_DANISH,
    IDM_LANGUAGE_GERMAN, IDM_LANGUAGE_SPANISH, IDM_LANGUAGE_FRENCH,
    IDM_LANGUAGE_HUNGARIAN, IDM_LANGUAGE_ITALIAN, IDM_LANGUAGE_KOREAN,
    IDM_LANGUAGE_DUTCH, IDM_LANGUAGE_POLISH, IDM_LANGUAGE_RUSSIAN,
    IDM_LANGUAGE_SWEDISH, IDM_LANGUAGE_TURKISH, IDM_LANGUAGE, IDM_ABOUT,
    IDM_CMDLINE, IDM_CONTRIBUTORS, IDM_LICENSE, IDM_WARRANTY,
    IDM_TOGGLE_USERPORT_DAC, IDM_CART_PLUS4_SMART_ATTACH, IDM_PSID_OVERRIDE,
    IDM_LOAD_PSID_FILE, IDM_NEXT_TUNE, IDM_PREVIOUS_TUNE, IDM_SAMPLE_RATE,
    IDM_BUFFER_SIZE, IDM_FRAGMENT_SIZE, IDM_SPEED_ADJUSTMENT, IDM_VOLUME,
    IDM_SOUND_OUTPUT_MODE, IDM_SID_ENGINE_MODEL, IDM_AMOUNT_OF_EXTRA_SIDS,
    IDM_SID_FILTERS, IDM_SAMPLE_METHOD
};

/* These have been altered for the locale support. */
#define TITLE(a, b) { NM_TITLE, (int)a, (STRPTR)b, 0, 0L, (APTR)NULL },
#define ITEM(a, b, c) { NM_ITEM, (int)a, (STRPTR)b, 0, 0L, (APTR)c },
#define SUB(a, b, c) { NM_SUB, (int)a, (STRPTR)b, 0, 0L, (APTR)c },
#define ITEMSEPARATOR() { NM_ITEM, (int)0, NULL, 0, 0L, NULL },
#define SUBSEPARATOR() { NM_SUB, (int)0, NULL, 0, 0L, NULL },
#define ITEMTOGGLE(a, b, c) { NM_ITEM, (int)a, (STRPTR)b, (CHECKIT | MENUTOGGLE), 0L, (APTR)c },
#define SUBTOGGLE(a, b, c) { NM_SUB, (int)a, (STRPTR)b, (CHECKIT | MENUTOGGLE), 0L, (APTR)c },
#define END() { NM_END, (int)0, NULL, 0, 0L, NULL },

struct TranslateNewMenu {
    UBYTE nm_Type;
    int nm_Label;
    STRPTR nm_CommKey;
    UWORD nm_Flags;
    LONG nm_MutualExclude;
    APTR nm_UserData;
};

extern void ui_register_menu_translation_layout(struct TranslateNewMenu *menu);

#endif /* _UIRES_H_ */
