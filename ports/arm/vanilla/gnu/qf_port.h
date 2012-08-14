//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++  port to ARM7/ARM9
// Last Updated for Version: 4.4.00
// Date of the Last Update:  Apr 19, 2012
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//
// Contact information:
// Quantum Leaps Web sites: http://www.quantum-leaps.com
//                          http://www.state-machine.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#ifndef qf_port_h
#define qf_port_h

                    // The maximum number of active objects in the application
#define QF_MAX_ACTIVE               63

                       // The maximum number of event pools in the application
#define QF_MAX_EPOOL                6

              // fast unconditional interrupt disabling/enabling for ARM state
#define QF_INT_DISABLE()   \
    __asm volatile ("MSR cpsr_c,#(0x1F | 0x80 | 0x40)" ::: "cc")

#define QF_INT_ENABLE() \
    __asm volatile ("MSR cpsr_c,#(0x1F)" ::: "cc")

                                             // QF critical section entry/exit
#ifdef __thumb__                                                // THUMB mode?
                                             // QF interrupt locking/unlocking
    #define QF_CRIT_STAT_TYPE         unsigned int
    #define QF_CRIT_ENTRY(stat_)      ((stat_) = QF_int_lock_SYS())
    #define QF_CRIT_EXIT(stat_)       QF_int_unlock_SYS(stat_)

    extern "C" QF_CRIT_STAT_TYPE QF_int_lock_SYS(void);
    extern "C" void QF_int_unlock_SYS(QF_CRIT_STAT_TYPE stat);

#elif (defined __arm__)                                           // ARM mode?

    #define QF_CRIT_STAT_TYPE       unsigned int
    #define QF_CRIT_ENTRY(stat_)    do { \
        __asm volatile ("MRS %0,cpsr" : "=r" (stat_) :: "cc"); \
        QF_INT_DISABLE(); \
    } while (0)
    #define QF_CRIT_EXIT(stat_) \
        __asm volatile ("MSR cpsr_c,%0" :: "r" (stat_) : "cc")

#else

    #error Incorrect CPU mode. Must be either __arm__ or __thumb__.

#endif

extern "C" {
    void QF_reset(void);
    void QF_undef(void);
    void QF_swi(void);
    void QF_pAbort(void);
    void QF_dAbort(void);
    void QF_reserved(void);
    void QF_irq(void);
    void QF_fiq(void);

    void BSP_irq(void);
    void BSP_fiq(void);
}

#include "qep_port.h"                                              // QEP port
#include "qvanilla.h"                          // "Vanilla" cooperative kernel
#include "qf.h"                    // QF platform-independent public interface

#endif                                                            // qf_port_h
