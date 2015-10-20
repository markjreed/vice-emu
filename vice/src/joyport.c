/*
 * joyport.c - joystick port handling.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include <string.h>

#include "cmdline.h"
#include "joyport.h"
#include "resources.h"
#include "translate.h"
#include "uiapi.h"

static joyport_t joyport_device[JOYPORT_MAX_DEVICES];

static int joy_port[2] = { JOYPORT_ID_NONE , JOYPORT_ID_NONE };
static int joy_pot_present = 0;
static int pot_port_mask = 1;
static int max_ports = 2;
static int joyport_set_done = 0;

void set_joyport_pot_mask(int mask)
{
    pot_port_mask = mask;
}

static int joyport_set_device(int port, int id)
{
    /* 1st some sanity checks */
    if (id < JOYPORT_ID_NONE || id >= JOYPORT_MAX_DEVICES) {
        return -1;
    }
    if (port < 0 || port > max_ports) {
        return -1;
    }

    /* Nothing changes */
    if (id == joy_port[port]) {
        return 0;
    }

    /* check if id is registered */
    if (id != JOYPORT_ID_NONE && !joyport_device[id].name) {
        ui_error(T_("Selected joyport device %d is not registered"), id);
        return -1;
    }

    /* check if id conflicts with device on the other port */
    if (id != JOYPORT_ID_NONE && max_ports != 1 && joy_port[!port] == id) {
        ui_error(T_("Selected joyport device %s on port %d is already attached to port %d"), joyport_device[id].name, port + 1, (!port) + 1);
        return -1;
    }

    /* check if input resource conflicts with device on the other port */
    if (id != JOYPORT_ID_NONE && max_ports != 1 && joyport_device[id].resource_id != JOYPORT_RES_ID_NONE && joyport_device[id].resource_id == joyport_device[joy_port[!port]].resource_id) {
        ui_error(T_("Selected joyport device %s on port %d uses same input resource as the device attached to port %d"), joyport_device[id].name, port + 1, (!port) + 1);
        return -1;
    }

    /* all checks done, now disable the current device and enable the new device */
    if (joyport_device[joy_port[port]].enable) {
        joyport_device[joy_port[port]].enable(0);
    }
    if (joyport_device[id].enable) {
        joyport_device[id].enable(id);
    }
    joy_port[port] = id;

    return 0;
}

BYTE read_joyport_dig(int port)
{
    int id = joy_port[port];

    if (id == JOYPORT_ID_NONE) {
        return 0xff;
    }

    if (!joyport_device[id].read_digital) {
        return 0xff;
    }
    return joyport_device[id].read_digital();
}

void store_joyport_dig(int port, BYTE val)
{
    int id = joy_port[port];

    if (id == JOYPORT_ID_NONE) {
        return;
    }

    if (!joyport_device[id].store_digital) {
        return;
    }

    joyport_device[id].store_digital(val);
}

BYTE read_joyport_potx(void)
{
    int id1 = JOYPORT_ID_NONE;
    int id2 = JOYPORT_ID_NONE;
    BYTE ret1 = 0xff;
    BYTE ret2 = 0xff;

    if (!joy_pot_present) {
        return 0xff;
    }

    if (pot_port_mask == 1 || pot_port_mask == 3) {
        id1 = joy_port[JOYPORT_1];
    }

    if (pot_port_mask == 2 || pot_port_mask == 3) {
        id2 = joy_port[JOYPORT_2];
    }

    if (id1 != JOYPORT_ID_NONE) {
        if (joyport_device[id1].read_potx) {
            ret1 = joyport_device[id1].read_potx();
        }
    }

    if (id2 != JOYPORT_ID_NONE) {
        if (joyport_device[id2].read_potx) {
            ret2 = joyport_device[id2].read_potx();
        }
    }

    switch (pot_port_mask) {
        case 1:
            return ret1;
        case 2:
            return ret2;
        case 3:
            return ret1 & ret2;
    }

    return 0xff;
}

BYTE read_joyport_poty(void)
{
    int id1 = JOYPORT_ID_NONE;
    int id2 = JOYPORT_ID_NONE;
    BYTE ret1 = 0xff;
    BYTE ret2 = 0xff;

    if (!joy_pot_present) {
        return 0xff;
    }

    if (pot_port_mask == 1 || pot_port_mask == 3) {
        id1 = joy_port[JOYPORT_1];
    }

    if (pot_port_mask == 2 || pot_port_mask == 3) {
        id2 = joy_port[JOYPORT_2];
    }

    if (id1 != JOYPORT_ID_NONE) {
        if (joyport_device[id1].read_poty) {
            ret1 = joyport_device[id1].read_poty();
        }
    }

    if (id2 != JOYPORT_ID_NONE) {
        if (joyport_device[id2].read_poty) {
            ret2 = joyport_device[id2].read_poty();
        }
    }

    switch (pot_port_mask) {
        case 1:
            return ret1;
        case 2:
            return ret2;
        case 3:
            return ret1 & ret2;
    }

    return 0xff;
}

int joyport_register(int id, joyport_t *device)
{
    if (!joyport_set_done) {
        joyport_set_done = 1;
        memset(joyport_device, 0, sizeof(joyport_device));
    }
    if (id < 1 || id > JOYPORT_MAX_DEVICES) {
        return -1;
    }
    joyport_device[id].name = device->name;
    joyport_device[id].resource_id = device->resource_id;
    joyport_device[id].enable = device->enable;
    joyport_device[id].read_digital = device->read_digital;
    joyport_device[id].store_digital = device->store_digital;
    joyport_device[id].read_potx = device->read_potx;
    joyport_device[id].read_poty = device->read_poty;
    return 0;
}

/* ------------------------------------------------------------------------- */

static int set_joyport1_device(int val, void *param)
{
    return joyport_set_device(0, val);
}

static int set_joyport2_device(int val, void *param)
{
    return joyport_set_device(1, val);
}

static const resource_int_t resources_int_port1[] = {
    { "JoyPort1Device", 0, RES_EVENT_NO, (resource_value_t)JOYPORT_ID_NONE,
      &joy_port[0], set_joyport1_device, NULL },
    { NULL }
};

static const resource_int_t resources_int_port2[] = {
    { "JoyPort2Device", 0, RES_EVENT_NO, (resource_value_t)JOYPORT_ID_NONE,
      &joy_port[1], set_joyport2_device, NULL },
    { NULL }
};

int joyport_resources_init(int pot_present, int ports)
{
    joy_pot_present = pot_present;
    max_ports = ports;

    if (!joyport_set_done) {
        joyport_set_done = 1;
        memset(joyport_device, 0, sizeof(joyport_device));
    }

    if (ports == JOYPORT_PORTS_2) {
        if (resources_register_int(resources_int_port2) < 0) {
            return -1;
        }
    }

    return resources_register_int(resources_int_port1);
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t cmdline_options_port1[] =
{
    { "-joyport1device", SET_RESOURCE, 1,
      NULL, NULL, "JoyPort1Device", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      T_("Device"), T_("Set joyport 1 device") },
    { NULL }
};

static const cmdline_option_t cmdline_options_port2[] =
{
    { "-joyport2device", SET_RESOURCE, 1,
      NULL, NULL, "JoyPort2Device", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      T_("Device"), T_("Set joyport 2 device") },
    { NULL }
};

int joyport_cmdline_options_init(void)
{
    if (max_ports == JOYPORT_PORTS_2) {
        if (cmdline_register_options(cmdline_options_port2) < 0) {
            return -1;
        }
    }

    return cmdline_register_options(cmdline_options_port1);
}
