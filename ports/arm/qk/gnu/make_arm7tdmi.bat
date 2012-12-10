@echo off
:: ==========================================================================
:: Product: QP/C++ buld script for ARM, QK port, GNU toolchain
:: Last Updated for Version: 4.5.02
:: Date of the Last Update:  Oct 07, 2012
::
::                    Q u a n t u m     L e a P s
::                    ---------------------------
::                    innovating embedded systems
::
:: Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
::
:: This program is open source software: you can redistribute it and/or
:: modify it under the terms of the GNU General Public License as published
:: by the Free Software Foundation, either version 2 of the License, or
:: (at your option) any later version.
::
:: Alternatively, this program may be distributed and modified under the
:: terms of Quantum Leaps commercial licenses, which expressly supersede
:: the GNU General Public License and are specifically designed for
:: licensees interested in retaining the proprietary status of their code.
::
:: This program is distributed in the hope that it will be useful,
:: but WITHOUT ANY WARRANTY; without even the implied warranty of
:: MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
:: GNU General Public License for more details.
::
:: You should have received a copy of the GNU General Public License
:: along with this program. If not, see <http:::www.gnu.org/licenses/>.
::
:: Contact information:
:: Quantum Leaps Web sites: http://www.quantum-leaps.com
::                          http://www.state-machine.com
:: e-mail:                  info@quantum-leaps.com
:: ==========================================================================
setlocal

:: define the GNU_ARM environment variable to point to the location 
:: where you've installed the GNU ARM toolset or adjust the following 
:: set instruction 
if "%GNU_ARM%"=="" set GNU_ARM=C:\tools\devkitPro\devkitARM

set PATH=%GNU_ARM%\bin;%PATH%

set CC=arm-none-eabi-gcc
set ASM=arm-none-eabi-as
set LIB=arm-none-eabi-ar

set QP_INCDIR=..\..\..\..\include
set QP_PRTDIR=.

set ARM_CORE=arm7tdmi

if "%1"=="" (
    echo default selected
    set BINDIR=dbg
    set CCFLAGS=-g -c -mcpu=%ARM_CORE% -mthumb-interwork -mlong-calls -ffunction-sections -Wall -O -fno-rtti -fno-exceptions
    set ASMFLAGS=-g -mcpu=%ARM_CORE% -mthumb-interwork
)
if "%1"=="rel" (
    echo rel selected
    set BINDIR=rel
    set CCFLAGS=-c -mcpu=%ARM_CORE% -mthumb-interwork -mlong-calls -ffunction-sections -Wall -Os -fno-rtti -fno-exceptions -DNDEBUG
    set ASMFLAGS=-mcpu=%ARM_CORE% -mthumb-interwork
)
if "%1"=="spy" (
    echo spy selected
    set BINDIR=spy
    set CCFLAGS=-g -c -mcpu=%ARM_CORE% -mthumb-interwork -mlong-calls -ffunction-sections -Wall -O -fno-rtti -fno-exceptions -DQ_SPY
    set ASMFLAGS=-g -mcpu=%ARM_CORE% -mthumb-interwork
)

mkdir %BINDIR%
set LIBDIR=%BINDIR%
set LIBFLAGS=rs
erase %LIBDIR%\libqp_%ARM_CORE%.a

:: QEP ----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qep\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qep.cpp      -o%BINDIR%\qep.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qfsm_ini.cpp -o%BINDIR%\qfsm_ini.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qfsm_dis.cpp -o%BINDIR%\qfsm_dis.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qhsm_ini.cpp -o%BINDIR%\qhsm_ini.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qhsm_dis.cpp -o%BINDIR%\qhsm_dis.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qhsm_top.cpp -o%BINDIR%\qhsm_top.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qhsm_in.cpp  -o%BINDIR%\qhsm_in.o

%LIB% %LIBFLAGS% %LIBDIR%\libqp_%ARM_CORE%.a %BINDIR%\qep.o %BINDIR%\qfsm_ini.o %BINDIR%\qfsm_dis.o %BINDIR%\qhsm_ini.o %BINDIR%\qhsm_dis.o %BINDIR%\qhsm_top.o %BINDIR%\qhsm_in.o
@echo off
erase %BINDIR%\*.o

:: QF -----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qf\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qa_defer.cpp -o%BINDIR%\qa_defer.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qa_fifo.cpp  -o%BINDIR%\qa_fifo.o 
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qa_lifo.cpp  -o%BINDIR%\qa_lifo.o 
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qa_get_.cpp  -o%BINDIR%\qa_get_.o 
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qa_sub.cpp   -o%BINDIR%\qa_sub.o  
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qa_usub.cpp  -o%BINDIR%\qa_usub.o 
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qa_usuba.cpp -o%BINDIR%\qa_usuba.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qeq_fifo.cpp -o%BINDIR%\qeq_fifo.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qeq_get.cpp  -o%BINDIR%\qeq_get.o 
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qeq_init.cpp -o%BINDIR%\qeq_init.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qeq_lifo.cpp -o%BINDIR%\qeq_lifo.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qf_act.cpp   -o%BINDIR%\qf_act.o  
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qf_gc.cpp    -o%BINDIR%\qf_gc.o      
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qf_log2.cpp  -o%BINDIR%\qf_log2.o 
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qf_new.cpp   -o%BINDIR%\qf_new.o  
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qf_pool.cpp  -o%BINDIR%\qf_pool.o 
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qf_psini.cpp -o%BINDIR%\qf_psini.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qf_pspub.cpp -o%BINDIR%\qf_pspub.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qf_pwr2.cpp  -o%BINDIR%\qf_pwr2.o 
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qf_tick.cpp  -o%BINDIR%\qf_tick.o 
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qmp_get.cpp  -o%BINDIR%\qmp_get.o 
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qmp_init.cpp -o%BINDIR%\qmp_init.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qmp_put.cpp  -o%BINDIR%\qmp_put.o 
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qte_ctor.cpp -o%BINDIR%\qte_ctor.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qte_arm.cpp  -o%BINDIR%\qte_arm.o 
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qte_darm.cpp -o%BINDIR%\qte_darm.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qte_rarm.cpp -o%BINDIR%\qte_rarm.o
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qte_ctr.cpp  -o%BINDIR%\qte_ctr.o
%ASM% src\qk_port.s -o %BINDIR%\qk_port.o %ASMFLAGS%

%LIB% %LIBFLAGS% %LIBDIR%\libqp_%ARM_CORE%.a %BINDIR%\qa_defer.o %BINDIR%\qa_fifo.o %BINDIR%\qa_lifo.o %BINDIR%\qa_get_.o %BINDIR%\qa_sub.o %BINDIR%\qa_usub.o %BINDIR%\qa_usuba.o %BINDIR%\qeq_fifo.o %BINDIR%\qeq_get.o %BINDIR%\qeq_init.o %BINDIR%\qeq_lifo.o %BINDIR%\qf_act.o %BINDIR%\qf_gc.o %BINDIR%\qf_log2.o %BINDIR%\qf_new.o %BINDIR%\qf_pool.o %BINDIR%\qf_psini.o %BINDIR%\qf_pspub.o %BINDIR%\qf_pwr2.o %BINDIR%\qf_tick.o %BINDIR%\qmp_get.o %BINDIR%\qmp_init.o %BINDIR%\qmp_put.o %BINDIR%\qte_ctor.o %BINDIR%\qte_arm.o %BINDIR%\qte_darm.o %BINDIR%\qte_rarm.o %BINDIR%\qte_ctr.o %BINDIR%\qk_port.o
@echo off
erase %BINDIR%\*.o

:: QK -----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qk\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qk.cpp       -o%BINDIR%\qk.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qk_sched.cpp -o%BINDIR%\qk_sched.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qk_mutex.cpp -o%BINDIR%\qk_mutex.o
%ASM% src\qk_port.s -o %BINDIR%\qk_port.o %ASMFLAGS%

%LIB% %LIBFLAGS% %LIBDIR%\libqp_%ARM_CORE%.a %BINDIR%\qk.o %BINDIR%\qk_sched.o %BINDIR%\qk_mutex.o %BINDIR%\qk_port.o
@echo off
erase %BINDIR%\*.o

:: QS -----------------------------------------------------------------------
if not "%1"=="spy" goto clean

set SRCDIR=..\..\..\..\qs\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

@echo on
%CC% -mthumb %CCFLAGS% %CCINC% %SRCDIR%\qs.cpp      -o%BINDIR%\qs.o     
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qs_.cpp     -o%BINDIR%\qs_.o     
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qs_blk.cpp  -o%BINDIR%\qs_blk.o 
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qs_byte.cpp -o%BINDIR%\qs_byte.o
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qs_f32.cpp  -o%BINDIR%\qs_f32.o 
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qs_f64.cpp  -o%BINDIR%\qs_f64.o 
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qs_mem.cpp  -o%BINDIR%\qs_mem.o 
%CC% -marm   %CCFLAGS% %CCINC% %SRCDIR%\qs_str.cpp  -o%BINDIR%\qs_str.o 

%LIB% %LIBFLAGS% %LIBDIR%\libqp_%ARM_CORE%.a %BINDIR%\qs.o %BINDIR%\qs_.o %BINDIR%\qs_blk.o %BINDIR%\qs_byte.o %BINDIR%\qs_f32.o %BINDIR%\qs_f64.o %BINDIR%\qs_mem.o %BINDIR%\qs_str.o
@echo off
erase %BINDIR%\*.o

:: --------------------------------------------------------------------------

:clean

endlocal

