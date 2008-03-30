/*
 * plus4-resources.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include "vice.h"

#include <stdio.h>

#include "drive.h"
#include "kbd.h"
#include "keyboard.h"
#include "lib.h"
#include "machine.h"
#include "mem.h"
#include "plus4mem.h"
#include "plus4rom.h"
#include "resources.h"
#include "utils.h"
#include "interrupt.h"
#include "vsync.h"


#define KBD_INDEX_PLUS4_SYM 0
#define KBD_INDEX_PLUS4_POS 1


/* What sync factor between the CPU and the drive?  If equal to
   `MACHINE_SYNC_PAL', the same as PAL machines.  If equal to
   `MACHINE_SYNC_NTSC', the same as NTSC machines.  The sync factor is
   calculated as 65536 * drive_clk / clk_[main machine] */
static int sync_factor;

/* Name of the BASIC ROM.  */
static char *basic_rom_name = NULL;

/* Name of the Kernal ROM.  */
static char *kernal_rom_name = NULL;

/* Name of the 3plus1 ROMs.  */
static char *tplus1lo_rom_name = NULL;
static char *tplus1hi_rom_name = NULL;

/* Size of RAM installed in kbytes */
static int ram_size_plus4 = 64;

static int set_kernal_rom_name(resource_value_t v, void *param)
{
    if (util_string_set(&kernal_rom_name, (const char *)v))
        return 0;

    return plus4rom_load_kernal(kernal_rom_name);
}

static int set_basic_rom_name(resource_value_t v, void *param)
{
    if (util_string_set(&basic_rom_name, (const char *)v))
        return 0;

    return plus4rom_load_basic(basic_rom_name);
}

static int set_3plus1lo_rom_name(resource_value_t v, void *param)
{
    if (util_string_set(&tplus1lo_rom_name, (const char *)v))
        return 0;

    return plus4rom_load_3plus1lo(tplus1lo_rom_name);
}

static int set_3plus1hi_rom_name(resource_value_t v, void *param)
{
    if (util_string_set(&tplus1hi_rom_name, (const char *)v))
        return 0;

    return plus4rom_load_3plus1hi(tplus1hi_rom_name);
}

static int set_ram_size_plus4(resource_value_t v, void *param)
{
    int rs = (int)v;

    if ((rs != 64) && (rs != 32) && (rs !=16)) {
        return -1;
    }
    ram_size_plus4 = rs;
    vsync_suspend_speed_eval();
    mem_initialize_memory();
    mem_powerup();
    maincpu_trigger_reset();
    return 0;
}

static int set_sync_factor(resource_value_t v, void *param)
{
    int change_timing = 0;

    if (sync_factor != (int)v)
        change_timing = 1;

    switch ((int)v) {
      case MACHINE_SYNC_PAL:
        sync_factor = (int)v;
        if (change_timing)
            machine_change_timing(MACHINE_SYNC_PAL);
        break;
      case MACHINE_SYNC_NTSC:
        sync_factor = (int)v;
        if (change_timing)
            machine_change_timing(MACHINE_SYNC_NTSC);
        break;
      default:
        return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

static const resource_t resources[] = {
    { "MachineVideoStandard", RES_INTEGER, (resource_value_t)MACHINE_SYNC_PAL,
      (resource_value_t *)&sync_factor,
      set_sync_factor, NULL },
    { "KernalName", RES_STRING, (resource_value_t)"kernal",
      (resource_value_t *)&kernal_rom_name,
      set_kernal_rom_name, NULL },
    { "BasicName", RES_STRING, (resource_value_t)"basic",
      (resource_value_t *)&basic_rom_name,
      set_basic_rom_name, NULL },
    { "3plus1loName", RES_STRING, (resource_value_t)"3plus1lo",
      (resource_value_t *)&tplus1lo_rom_name,
      set_3plus1lo_rom_name, NULL },
    { "3plus1hiName", RES_STRING, (resource_value_t)"3plus1hi",
      (resource_value_t *)&tplus1hi_rom_name,
      set_3plus1hi_rom_name, NULL },
    { "RamSize", RES_INTEGER, (resource_value_t)64,
      (resource_value_t *)&ram_size_plus4,
      set_ram_size_plus4, NULL },
#ifdef COMMON_KBD
    { "KeymapIndex", RES_INTEGER, (resource_value_t)KBD_INDEX_PLUS4_DEFAULT,
      (resource_value_t *)&machine_keymap_index,
      keyboard_set_keymap_index, NULL },
    { "KeymapSymFile", RES_STRING,
      (resource_value_t)KBD_PLUS4_SYM,
      (resource_value_t *)&machine_keymap_file_list[0],
      keyboard_set_keymap_file, (void *)0 },
    { "KeymapPosFile", RES_STRING,
      (resource_value_t)KBD_PLUS4_POS,
      (resource_value_t *)&machine_keymap_file_list[1],
      keyboard_set_keymap_file, (void *)1 },
#endif
    { NULL }
};

int plus4_resources_init(void)
{
    return resources_register(resources);
}

void plus4_resources_shutdown(void)
{
    lib_free(basic_rom_name);
    lib_free(kernal_rom_name);
    lib_free(tplus1lo_rom_name);
    lib_free(tplus1hi_rom_name);
    lib_free(machine_keymap_file_list[0]);
    lib_free(machine_keymap_file_list[1]);
}

