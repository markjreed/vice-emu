/*
 * maincpu.c - Emulation of the main 6510 processor.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include "6510core.h"
#include "alarm.h"
#include "clkguard.h"
#include "debug.h"
#include "interrupt.h"
#include "log.h"
#include "machine.h"
#include "maincpu.h"
#include "mem.h"
#include "mon.h"
#include "mos6510.h"
#include "snapshot.h"
#include "traps.h"
#include "types.h"
#include "utils.h"

/* ------------------------------------------------------------------------- */

/* MACHINE_STUFF should define/undef

 - NEED_REG_PC

 The following are optional:

 - PAGE_ZERO
 - PAGE_ONE
 - STORE_IND
 - LOAD_IND

*/

/* This enables a special hack that can speed up the instruction fetch quite
   a lot, but does not work when a conditional branch instruction jumps from
   ROM to RAM or vice versa.  It still works if one program lies in the I/O
   space, though.  Keeping this defined should be OK for everything, and
   makes things much faster.

   This define affects only this file! */
# ifndef NO_INSTRUCTION_FETCH_HACK
#  define INSTRUCTION_FETCH_HACK
# endif
# ifndef INSTRUCTION_FETCH_HACK
/* Define a "special" opcode fetch method.  We trust the code in `6510core.c'
   to evaluate `p0', `p1' and `p2' at most once per every emulated opcode.  */
#  define FETCH_OPCODE(x)
#  define p0                    LOAD(reg_pc)
#  define p1                    LOAD(reg_pc + 1)
#  define p2                    LOAD_ADDR(reg_pc + 1)

/* FIXME: This might cause complaints about unused variables...  Well, who
   cares?  */
#  define opcode_t      int
# endif

/* ------------------------------------------------------------------------- */

/* The following #defines are useful for debugging and speed tuning.  */

/* Use a global variable for Program Counter.  This makes it slower, but also
   makes debugging easier.  This is needed by the VIC-II emulation, so avoid
   #undefining or #defining it in case it is already #defined.  */
#if !defined EXTERN_PC
#  undef EXTERN_PC
#endif

/* ------------------------------------------------------------------------- */

/* Now, if EXTERN_PC is set, set NEED_REG_PC, because it's this one
   that's being used.  Do not change this one.  */
#ifdef EXTERN_PC
#  define NEED_REG_PC
#endif

/* ------------------------------------------------------------------------- */

/* Implement the hack to make opcode fetches faster.  */
#ifdef INSTRUCTION_FETCH_HACK

#  define JUMP(addr)                         \
     do {                                    \
       reg_pc = (addr);                      \
       bank_base = mem_read_base(reg_pc);    \
       bank_limit = mem_read_limit(reg_pc);  \
       mem_old_reg_pc = reg_pc;              \
     } while (0)

#else  /* !INSTRUCTION_FETCH_HACK */

#  define JUMP(addr)    (reg_pc = (addr))

#endif /* !INSTRUCTION_FETCH_HACK */

/* ------------------------------------------------------------------------- */

#ifndef STORE
#define STORE(addr, value) \
    (*_mem_write_tab_ptr[(addr) >> 8])((ADDRESS)(addr), (BYTE)(value))
#endif

#ifndef LOAD
#define LOAD(addr) \
    (*_mem_read_tab_ptr[(addr) >> 8])((ADDRESS)(addr))
#endif

#ifndef STORE_ZERO
#define STORE_ZERO(addr, value) \
    store_zero((ADDRESS)(addr), (BYTE)(value))
#endif

#ifndef LOAD_ZERO
#define LOAD_ZERO(addr) \
    PAGE_ZERO[(addr) & 0xff]
#endif

#define LOAD_ADDR(addr) \
    ((LOAD((addr) + 1) << 8) | LOAD(addr))

#define LOAD_ZERO_ADDR(addr) \
    ((LOAD_ZERO((addr) + 1) << 8) | LOAD_ZERO(addr))

inline static BYTE *mem_read_base(int addr)
{
    BYTE *p = _mem_read_base_tab_ptr[addr >> 8];

    if (p == NULL)
        return p;

    return p - (addr & 0xff00);
}

inline static int mem_read_limit(int addr)
{
    return mem_read_limit_tab_ptr[addr >> 8];
}

void maincpu_generic_dma(void)
{
    /* Generic DMA hosts can be implemented here.
       For example a very accurate REU emulation. */
}

/* Those may be overridden by the machine stuff.  Probably we want them in
   the .def files, but if most of the machines do not use, we might keep it
   here and only override it where needed.  */
#ifndef PAGE_ZERO
#define PAGE_ZERO mem_ram
#endif

#ifndef PAGE_ONE
#define PAGE_ONE (mem_ram + 0x100)
#endif

#ifndef STORE_IND
#define STORE_IND(addr, value) STORE((addr),(value))
#endif

#ifndef LOAD_IND
#define LOAD_IND(addr) LOAD((addr))
#endif

#ifndef DMA_FUNC
#define DMA_FUNC maincpu_generic_dma();
#endif

#ifndef DMA_ON_RESET
#define DMA_ON_RESET
#endif

/* ------------------------------------------------------------------------- */

struct cpu_int_status_s maincpu_int_status;
alarm_context_t *maincpu_alarm_context;
clk_guard_t maincpu_clk_guard;

/* Global clock counter.  */
CLOCK maincpu_clk = 0L;

/* This is flag is set to 1 each time a Read-Modify-Write instructions that
   accesses memory is executed.  We can emulate the RMW behaviour of the 6510
   this way.  VERY important notice: Always assign 1 for true, 0 for false!
   Some functions depend on this to do some optimization.  */
int maincpu_rmw_flag = 0;

/* Information about the last executed opcode.  This is used to know the
   number of write cycles in the last executed opcode and to delay interrupts
   by one more cycle if necessary, as happens with conditional branch opcodes
   when the branch is taken.  */
DWORD last_opcode_info;

/* Number of write cycles for each 6510 opcode.  */
CLOCK _maincpu_opcode_write_cycles[] = {
            /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
    /* $00 */  3, 0, 0, 2, 0, 0, 2, 2, 1, 0, 0, 0, 0, 0, 2, 2, /* $00 */
    /* $10 */  0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 2, 2, /* $10 */
    /* $20 */  2, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, /* $20 */
    /* $30 */  0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 2, 2, /* $30 */
    /* $40 */  0, 0, 0, 2, 0, 0, 2, 2, 1, 0, 0, 0, 0, 0, 2, 2, /* $40 */
    /* $50 */  0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 2, 2, /* $50 */
    /* $60 */  0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, /* $60 */
    /* $70 */  0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 2, 2, /* $70 */
    /* $80 */  0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, /* $80 */
    /* $90 */  0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, /* $90 */
    /* $A0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* $A0 */
    /* $B0 */  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* $B0 */
    /* $C0 */  0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, /* $C0 */
    /* $D0 */  0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 2, 2, /* $D0 */
    /* $E0 */  0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 2, 2, /* $E0 */
    /* $F0 */  0, 0, 0, 2, 0, 0, 2, 2, 0, 0, 0, 2, 0, 0, 2, 2  /* $F0 */
            /* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
};

/* Public copy of the CPU registers.  As putting the registers into the
   function makes it faster, you have to generate a `TRAP' interrupt to have
   the values copied into this struct.  */
mos6510_regs_t maincpu_regs;

/* ------------------------------------------------------------------------- */

/* Interface to the monitor.  */
monitor_interface_t maincpu_monitor_interface = {

    /* Pointer to the registers of the CPU.  */
    &maincpu_regs,

#ifdef HAVE_Z80_REGS
    &z80_regs,
#else
    NULL,
#endif

    /* Pointer to the alarm/interrupt status.  */
    &maincpu_int_status,

    /* Pointer to the machine's clock counter.  */
    &maincpu_clk,

    0,
    mem_bank_list,
    mem_bank_from_name,
    mem_bank_read,
    mem_bank_peek,
    mem_bank_write,

    mem_ioreg_list_get,

    /* Pointer to a function to disable/enable watchpoint checking.  */
    mem_toggle_watchpoints,

    NULL
};

/* ------------------------------------------------------------------------- */

static void clk_overflow_callback(CLOCK sub, void *data)
{
    alarm_context_time_warp(maincpu_alarm_context, sub, -1);
    interrupt_cpu_status_time_warp(&maincpu_int_status, sub, -1);
}

void maincpu_init(void)
{
    maincpu_alarm_context = (alarm_context_t *)xmalloc(sizeof(alarm_context_t));

    alarm_context_init(maincpu_alarm_context, "MainCPU");

    clk_guard_init(&maincpu_clk_guard, &maincpu_clk, CLOCK_MAX - 0x100000);
    clk_guard_add_callback(&maincpu_clk_guard, clk_overflow_callback, NULL);
}

/* ------------------------------------------------------------------------- */

static void cpu_reset(void)
{
    int preserve_monitor;

    preserve_monitor = maincpu_int_status.global_pending_int & IK_MONITOR;

    log_message(LOG_DEFAULT, "Main CPU: RESET.");

    interrupt_cpu_status_init(&maincpu_int_status, NUMOFINT, &last_opcode_info);
    if (preserve_monitor)
        interrupt_monitor_trap_on(&maincpu_int_status);

    maincpu_clk = 6; /* # of clock cycles needed for RESET.  */

    /* Do machine-specific initialization.  */
    machine_reset();
}

void maincpu_reset(void)
{
    cpu_reset();
}

/* ------------------------------------------------------------------------- */

#ifdef NEED_REG_PC
unsigned int reg_pc;
#endif

void maincpu_mainloop(void)
{
    /* Notice that using a struct for these would make it a lot slower (at
       least, on gcc 2.7.2.x).  */
    BYTE reg_a = 0;
    BYTE reg_x = 0;
    BYTE reg_y = 0;
    BYTE reg_p = 0;
    BYTE reg_sp = 0;
    BYTE flag_n = 0;
    BYTE flag_z = 0;
#ifndef NEED_REG_PC
    unsigned int reg_pc;
#endif

#ifdef INSTRUCTION_FETCH_HACK
    BYTE *bank_base;
    int bank_limit;

    mem_set_bank_pointer(&bank_base, &bank_limit);
#endif

    maincpu_trigger_reset();
    log_message(LOG_DEFAULT, "Main CPU: starting at ($FFFC).");

    while (1) {

#define CLK maincpu_clk
#define RMW_FLAG maincpu_rmw_flag
#define LAST_OPCODE_INFO last_opcode_info
#define TRACEFLG debug.maincpu_traceflg

#define CPU_INT_STATUS maincpu_int_status

#define ALARM_CONTEXT maincpu_alarm_context

#define CHECK_PENDING_ALARM() \
   (clk >= next_alarm_clk(&maincpu_int_status))

#define CHECK_PENDING_INTERRUPT() \
   check_pending_interrupt(&maincpu_int_status)

#define TRAP(addr) \
   maincpu_int_status.trap_func(addr);

#define ROM_TRAP_HANDLER() \
   traps_handler()

#define JAM()                                                       \
   do {                                                             \
      unsigned int tmp;                                             \
                                                                    \
      EXPORT_REGISTERS();                                           \
      tmp = machine_jam("   " CPU_STR ": JAM at $%04X   ", reg_pc); \
      switch (tmp) {                                                \
        case JAM_RESET:                                             \
          DO_INTERRUPT(IK_RESET);                                   \
          break;                                                    \
        case JAM_HARD_RESET:                                        \
          mem_powerup();                                            \
          DO_INTERRUPT(IK_RESET);                                   \
          break;                                                    \
        case JAM_MONITOR:                                           \
          caller_space = e_comp_space;                              \
          mon(reg_pc);                                              \
          IMPORT_REGISTERS();                                       \
          break;                                                    \
        default:                                                    \
          CLK++;                                                    \
      }                                                             \
   } while (0)

#define CALLER e_comp_space

#define ROM_TRAP_ALLOWED() mem_rom_trap_allowed(reg_pc)

#define GLOBAL_REGS maincpu_regs

#include "6510core.c"

    }
}

/* ------------------------------------------------------------------------- */

static char snap_module_name[] = "MAINCPU";
#define SNAP_MAJOR 1
#define SNAP_MINOR 0

int maincpu_snapshot_write_module(snapshot_t *s)
{
    snapshot_module_t *m;

    m = snapshot_module_create(s, snap_module_name, ((BYTE)SNAP_MAJOR),
                               ((BYTE)SNAP_MINOR));
    if (m == NULL)
        return -1;

    if (0
        || snapshot_module_write_dword(m, maincpu_clk) < 0
        || snapshot_module_write_byte(m, MOS6510_REGS_GET_A(&maincpu_regs)) < 0
        || snapshot_module_write_byte(m, MOS6510_REGS_GET_X(&maincpu_regs)) < 0
        || snapshot_module_write_byte(m, MOS6510_REGS_GET_Y(&maincpu_regs)) < 0
        || snapshot_module_write_byte(m, MOS6510_REGS_GET_SP(&maincpu_regs)) < 0
        || snapshot_module_write_word(m, MOS6510_REGS_GET_PC(&maincpu_regs)) < 0
        || snapshot_module_write_byte(m, MOS6510_REGS_GET_STATUS(&maincpu_regs)) < 0
        || snapshot_module_write_dword(m, (DWORD)(last_opcode_info)) < 0
        )
        goto fail;

    if (interrupt_write_snapshot(&maincpu_int_status, m) < 0)
        goto fail;

    return snapshot_module_close(m);

fail:
    if (m != NULL)
        snapshot_module_close(m);
    return -1;
}

int maincpu_snapshot_read_module(snapshot_t *s)
{
    BYTE a, x, y, sp, status;
    WORD pc;
    BYTE major, minor;
    snapshot_module_t *m;

    m = snapshot_module_open(s, snap_module_name, &major, &minor);
    if (m == NULL)
        return -1;

    /* FIXME: This is a mighty kludge to prevent VIC-II from stealing the
       wrong number of cycles.  */
    maincpu_rmw_flag = -1;

    /* XXX: Assumes `CLOCK' is the same size as a `DWORD'.  */
    if (0
        || snapshot_module_read_dword(m, &maincpu_clk) < 0
        || snapshot_module_read_byte(m, &a) < 0
        || snapshot_module_read_byte(m, &x) < 0
        || snapshot_module_read_byte(m, &y) < 0
        || snapshot_module_read_byte(m, &sp) < 0
        || snapshot_module_read_word(m, &pc) < 0
        || snapshot_module_read_byte(m, &status) < 0
        || snapshot_module_read_dword(m, &last_opcode_info) < 0
        )
        goto fail;

    MOS6510_REGS_SET_A(&maincpu_regs, a);
    MOS6510_REGS_SET_X(&maincpu_regs, x);
    MOS6510_REGS_SET_Y(&maincpu_regs, y);
    MOS6510_REGS_SET_SP(&maincpu_regs, sp);
    MOS6510_REGS_SET_PC(&maincpu_regs, pc);
    MOS6510_REGS_SET_STATUS(&maincpu_regs, status);

    if (interrupt_read_snapshot(&maincpu_int_status, m) < 0)
        goto fail;

    return snapshot_module_close(m);

fail:
    if (m != NULL)
        snapshot_module_close(m);
    return -1;
}

