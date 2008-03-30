/*
 * dialogs.c - The dialogs.
 *
 * Written by
 *  Thomas Bretz <tbretz@gsi.de>
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

#define INCL_WINDIALOGS

#include "vice.h"
#include "dialogs.h"

#include "resources.h"

int toggle(const char *resource_name)
{
    int val;
    if (resources_toggle(resource_name, (resource_value_t*)&val)<0)
        return -1;
    return val;
}

/* call to open dialog                                              */
/*----------------------------------------------------------------- */
void about_dialog(HWND hwnd)
{
    static HWND hwnd2 = NULLHANDLE;

    if (WinIsWindowVisible(hwnd2))
        return;

    hwnd2 = WinLoadDlg(HWND_DESKTOP, hwnd, WinDefDlgProc, NULLHANDLE,
                       DLG_ABOUT, NULL);
}

