//////////////////////////////////////////////////////////////////////////////
// Product: QF/C++ port, ColdFire, QK kernel, IAR Compiler
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
#define QF_MAX_ACTIVE       63

#define QF_EVENT_SIZ_SIZE   2
#define QF_EQUEUE_CTR_SIZE  2
#define QF_MPOOL_SIZ_SIZE   2
#define QF_MPOOL_CTR_SIZE   2
#define QF_TIMEEVT_CTR_SIZE 4

                                             // QF critical section entry/exit
   // QF_INT_KEY_TYPE defined--"saving and restoring interrupt status" policy
#define QF_INT_KEY_TYPE     uint16_t
#define QF_INT_LOCK(key_)   do { \
    (key_) = __get_status_register(); \
    __disable_interrupts(); \
} while (0)
#define QF_INT_UNLOCK(key_) __set_status_register(key_)

#include <intrinsics.h>                             // IAR intrinsic functions

#include "qep_port.h"                                              // QEP port
#include "qk_port.h"                                                // QK port
#include "qf.h"                    // QF platform-independent public interface

#endif                                                            // qf_port_h
