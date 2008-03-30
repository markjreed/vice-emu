/*
 * ieeerom.c
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

#include "drive.h"
#include "driverom.h"
#include "ieeerom.h"


int ieeerom_check_loaded(unsigned int type)
{
    switch (type) {
      case DRIVE_TYPE_2031:
        if (rom2031_loaded < 1 && rom_loaded)
            return -1;
        break;
      case DRIVE_TYPE_2040:
        if (rom2040_loaded < 1 && rom_loaded)
            return -1;
        break;
      case DRIVE_TYPE_3040:
        if (rom3040_loaded < 1 && rom_loaded)
            return -1;
        break;
      case DRIVE_TYPE_4040:
        if (rom4040_loaded < 1 && rom_loaded)
            return -1;
        break;
      case DRIVE_TYPE_1001:
      case DRIVE_TYPE_8050:
      case DRIVE_TYPE_8250:
        if (rom1001_loaded < 1 && rom_loaded)
            return -1;
        break;
      default:
        return -1;
    }

    return 0;
}

