/*
 * initcmdline.c - Initial command line options.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "attach.h"
#include "autostart.h"
#include "charset.h"
#include "cmdline.h"
#include "initcmdline.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "tape.h"
#ifdef HAS_TRANSLATION
#include "translate.h"
#endif
#include "util.h"


static char *autostart_string = NULL;
static char *startup_disk_images[4];
static char *startup_tape_image;
static unsigned int autostart_mode;


static void cmdline_free_autostart_string(void)
{
    if (autostart_string != NULL)
        lib_free(autostart_string);
    autostart_string = NULL;
}

static int cmdline_help(const char *param, void *extra_param)
{
    cmdline_show_help(NULL);
    exit(0);

    return 0;   /* OSF1 cc complains */
}

static int cmdline_default(const char *param, void *extra_param)
{
    return resources_set_defaults();
}

static int cmdline_autostart(const char *param, void *extra_param)
{
    cmdline_free_autostart_string();
    autostart_string = lib_stralloc(param);
    autostart_mode = AUTOSTART_MODE_RUN;
    return 0;
}

static int cmdline_autoload(const char *param, void *extra_param)
{
    cmdline_free_autostart_string();
    autostart_string = lib_stralloc(param);
    autostart_mode = AUTOSTART_MODE_LOAD;
    return 0;
}

#ifndef __OS2__
static int cmdline_console(const char *param, void *extra_param)
{
    console_mode = 1;
    return 0;
}
#endif


static int cmdline_attach(const char *param, void *extra_param)
{
    int unit = (int)extra_param;

    switch (unit) {
      case 1:
        if (startup_tape_image != NULL)
            lib_free(startup_tape_image);
        startup_tape_image = lib_stralloc(param);
        break;
      case 8:
      case 9:
      case 10:
      case 11:
        if (startup_disk_images[unit - 8] != NULL)
            lib_free(startup_disk_images[unit - 8]);
        startup_disk_images[unit - 8] = lib_stralloc(param);
        break;
      default:
        archdep_startup_log_error("cmdline_attach(): unexpected unit number %d?!\n",
                                  unit);
    }

    return 0;
}

#ifdef HAS_TRANSLATION
static const cmdline_option_trans_t common_cmdline_options[] = {
    { "-help", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      0, IDCLS_SHOW_COMMAND_LINE_OPTIONS },
    { "-?", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      0, IDCLS_SHOW_COMMAND_LINE_OPTIONS },
    { "-h", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      0, IDCLS_SHOW_COMMAND_LINE_OPTIONS },
#if (!defined  __OS2__ && !defined __BEOS__)
    { "-console", CALL_FUNCTION, 0, cmdline_console, NULL, NULL, NULL,
      0, IDCLS_CONSOLE_MODE },
    { "-core", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)1,
      0, IDCLS_ALLOW_CORE_DUMPS },
    { "+core", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)0,
      0, IDCLS_DONT_ALLOW_CORE_DUMPS },
#else
    { "-debug", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)1,
      0, IDCLS_DONT_CALL_EXCEPTION_HANDLER },
    { "+debug", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)0,
      0, IDCLS_CALL_EXCEPTION_HANDLER },
#endif
    { NULL }
};
#else
static const cmdline_option_t common_cmdline_options[] = {
    { "-help", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      NULL, "Show a list of the available options and exit normally" },
    { "-?", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      NULL, "Show a list of the available options and exit normally" },
    { "-h", CALL_FUNCTION, 0, cmdline_help, NULL, NULL, NULL,
      NULL, "Show a list of the available options and exit normally" },
#if (!defined  __OS2__ && !defined __BEOS__)
    { "-console", CALL_FUNCTION, 0, cmdline_console, NULL, NULL, NULL,
      NULL, "Console mode (for music playback)" },
    { "-core", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)1,
      NULL, "Allow production of core dumps" },
    { "+core", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)0,
      NULL, "Do not produce core dumps" },
#else
    { "-debug", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)1,
      NULL, "Don't call exception handler" },
    { "+debug", SET_RESOURCE, 0, NULL, NULL, "DoCoreDump", (resource_value_t)0,
      NULL, "Call exception handler (default)" },
#endif
    { NULL }
};
#endif

#ifdef HAS_TRANSLATION
static const cmdline_option_trans_t vsid_cmdline_options[] = {
#else
static const cmdline_option_t vsid_cmdline_options[] = {
#endif
    { NULL }
};

/* These are the command-line options for the initialization sequence.  */

#ifdef HAS_TRANSLATION
static const cmdline_option_trans_t cmdline_options[] = {
    { "-default", CALL_FUNCTION, 0, cmdline_default, NULL, NULL, NULL,
      0, IDCLS_RESTORE_DEFAULT_SETTINGS },
    { "-autostart", CALL_FUNCTION, 1, cmdline_autostart, NULL, NULL, NULL,
      IDCLS_P_NAME, IDCLS_ATTACH_AND_AUTOSTART },
    { "-autoload", CALL_FUNCTION, 1, cmdline_autoload, NULL, NULL, NULL,
      IDCLS_P_NAME, IDCLS_ATTACH_AND_AUTOLOAD },
    { "-1", CALL_FUNCTION, 1, cmdline_attach, (void *)1, NULL, NULL,
      IDCLS_P_NAME, IDCLS_ATTACH_AS_TAPE },
    { "-8", CALL_FUNCTION, 1, cmdline_attach, (void *)8, NULL, NULL,
      IDCLS_P_NAME, IDCLS_ATTACH_AS_DISK_8 },
    { "-9", CALL_FUNCTION, 1, cmdline_attach, (void *)9, NULL, NULL,
      IDCLS_P_NAME, IDCLS_ATTACH_AS_DISK_9 },
    { "-10", CALL_FUNCTION, 1, cmdline_attach, (void *)10, NULL, NULL,
      IDCLS_P_NAME, IDCLS_ATTACH_AS_DISK_10 },
    { "-11", CALL_FUNCTION, 1, cmdline_attach, (void *)11, NULL, NULL,
      IDCLS_P_NAME, IDCLS_ATTACH_AS_DISK_11 },
    { NULL }
};
#else
static const cmdline_option_t cmdline_options[] = {
    { "-default", CALL_FUNCTION, 0, cmdline_default, NULL, NULL, NULL,
      NULL, "Restore default (factory) settings" },
    { "-autostart", CALL_FUNCTION, 1, cmdline_autostart, NULL, NULL, NULL,
      "<name>", "Attach and autostart tape/disk image <name>" },
    { "-autoload", CALL_FUNCTION, 1, cmdline_autoload, NULL, NULL, NULL,
      "<name>", "Attach and autoload tape/disk image <name>" },
    { "-1", CALL_FUNCTION, 1, cmdline_attach, (void *)1, NULL, NULL,
      "<name>", "Attach <name> as a tape image" },
    { "-8", CALL_FUNCTION, 1, cmdline_attach, (void *)8, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #8" },
    { "-9", CALL_FUNCTION, 1, cmdline_attach, (void *)9, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #9" },
    { "-10", CALL_FUNCTION, 1, cmdline_attach, (void *)10, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #10" },
    { "-11", CALL_FUNCTION, 1, cmdline_attach, (void *)11, NULL, NULL,
      "<name>", "Attach <name> as a disk image in drive #11" },
    { NULL }
};
#endif

int initcmdline_init(void)
{
#ifdef HAS_TRANSLATION
    const cmdline_option_trans_t *main_cmdline_options =
#else
    const cmdline_option_t *main_cmdline_options =
#endif
        vsid_mode ? vsid_cmdline_options : cmdline_options;

#ifdef HAS_TRANSLATION
    if (cmdline_register_options_trans(common_cmdline_options) < 0)
#else
    if (cmdline_register_options(common_cmdline_options) < 0)
#endif
        return -1;

#ifdef HAS_TRANSLATION
    if (cmdline_register_options_trans(main_cmdline_options) < 0)
#else
    if (cmdline_register_options(main_cmdline_options) < 0)
#endif
        return -1;

    return 0;
}

int initcmdline_check_psid(void)
{
    /* Check for PSID here since we don't want to allow autodetection
       in autostart.c. */
    if (vsid_mode) {
        if (autostart_string != NULL
            && machine_autodetect_psid(autostart_string) == -1) {
            log_error(LOG_DEFAULT, "`%s' is not a valid PSID file.",
                      autostart_string);
            return -1;
        }
    }

    return 0;
}

int initcmdline_check_args(int argc, char **argv)
{
    if (cmdline_parse(&argc, argv) < 0) {
        archdep_startup_log_error("Error parsing command-line options, bailing out. For help use '-help'\n");
        return -1;
    }

    /* The last orphan option is the same as `-autostart'.  */
    if (argc >= 1 && autostart_string == NULL) {
        autostart_string = lib_stralloc(argv[1]);
        argc--, argv++;
    }

    if (argc > 1) {
        int len = 0, j;

        for (j = 1; j < argc; j++)
            len += strlen(argv[j]);

        {
            char *txt = lib_calloc(1, len + argc + 1);
            for (j = 1; j < argc; j++)
                strcat(strcat(txt, " "), argv[j]);
            archdep_startup_log_error("Extra arguments on command-line: %s\n",
                                      txt);
            lib_free(txt);
        }
        return -1;
    }

    return 0;
}

/* These are a helper function for the `-autostart' command-line option.  It
   replaces all the $[0-9A-Z][0-9A-Z] patterns in `string' and returns it.  */
static char *hexstring_to_byte(const char *s, BYTE *value_return)
{
    int count;
    char c;

    for (*value_return = 0, count = 0; count < 2 && *s != 0; s++) {
        c = toupper(*s);
        if (c >= 'A' && c <= 'F') {
            *value_return <<= 4;
            *value_return += c - 'A' + 10;
        } else if (isdigit (c)) {
            *value_return <<= 4;
            *value_return += c - '0';
        } else {
            return (char *) s;
        }
    }

    return (char *)s;
}

static char *replace_hexcodes(char *s)
{
    unsigned int len, dest_len;
    char *p;
    char *new_s;

    len = strlen(s);
    new_s = (char*)lib_malloc(len + 1);

    p = s;
    dest_len = 0;
    while (1) {
        char *p1;

        p1 = strchr (p, '$');
        if (p1 != NULL) {
            char *new_p;

            new_p = hexstring_to_byte(p1 + 1,
                                      (BYTE *)(new_s + dest_len + (p1 - p)));
            if (p1 != p) {
                memcpy(new_s + dest_len, p, (size_t)(p1 - p));
                dest_len += p1 - p;
                dest_len++;
            }

            p = new_p;
        } else {
            break;
        }
    }

    strcpy((char *)(new_s + dest_len), p);
    return new_s;
}

void initcmdline_check_attach(void)
{
    if (!vsid_mode) {
        /* Handle general-purpose command-line options.  */

        /* `-autostart' */
        if (autostart_string != NULL) {
            char *tmp;

            /* Check for image:prg -format.  */
            tmp = strrchr(autostart_string, ':');
            if (tmp) {
                char *autostart_prg_name;
                char *autostart_file;

                autostart_file = lib_stralloc(autostart_string);
                autostart_prg_name = strrchr(autostart_file, ':');
                *autostart_prg_name++ = '\0';
                /* Does the image exist?  */
                if (util_file_exists(autostart_file)) {
                    char *name;

                    charset_petconvstring((BYTE *)autostart_prg_name, 0);
                    name = replace_hexcodes(autostart_prg_name);
                    autostart_autodetect(autostart_file, name, 0,
                                         autostart_mode);
                    lib_free(name);
                } else
                    autostart_autodetect(autostart_string, NULL, 0,
                                         autostart_mode);
                lib_free(autostart_file);
            } else {
                autostart_autodetect(autostart_string, NULL, 0,
                                     autostart_mode);
            }
        }
        /* `-8', `-9', `-10' and `-11': Attach specified disk image.  */
        {
            int i;

            for (i = 0; i < 4; i++) {
                if (startup_disk_images[i] != NULL
                    && file_system_attach_disk(i + 8, startup_disk_images[i])
                    < 0)
                    log_error(LOG_DEFAULT,
                              "Cannot attach disk image `%s' to unit %d.",
                              startup_disk_images[i], i + 8);
            }
        }

        /* `-1': Attach specified tape image.  */
        if (startup_tape_image && tape_image_attach(1, startup_tape_image) < 0)
            log_error(LOG_DEFAULT, "Cannot attach tape image `%s'.",
                      startup_tape_image);

    }

    cmdline_free_autostart_string();
}

