@echo off
:: ===========================================================================
:: Product: QP/C++ buld script for MSP430, Vanilla port, IAR compiler
:: Last Updated for Version: 4.4.00
:: Date of the Last Update:  Apr 19, 2012
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
:: ===========================================================================
setlocal

:: define the IAR_430 environment variable to point to the location 
:: where you've installed the IAR toolset or adjust the following 
:: set instruction 
if "%IAR_430%"=="" set IAR_430=C:\tools\IAR\430_KS_5.30

:: Typically, you don't need to modify this file past this line -------------

set PATH=%IAR_430%\430\bin;%IAR_430%\common\bin;%PATH%

set CC=icc430
set ASM=a430
set LIB=xar

set QP_INCDIR=..\..\..\..\include
set QP_PRTDIR=.

if "%1"=="" (
    echo default selected
    set BINDIR=dbg
    set CCFLAGS=-I%IAR_430%\430\inc -I%IAR_430%\430\inc\dlib -Ohz --debug --eec++ -e --double=32 --reduce_stack_usage --dlib_config %IAR_430%\430\lib\dlib\dl430fn.h --diag_suppress Pa050
)
if "%1"=="rel" (
    echo rel selected
    set BINDIR=rel
    set CCFLAGS=-I%IAR_430%\430\inc -I%IAR_430%\430\inc\dlib -Ohz --eec++ -e --double=32 --reduce_stack_usage --dlib_config %IAR_430%\430\lib\dlib\dl430fn.h --diag_suppress Pa050 -DNDEBUG
)
if "%1"=="spy" (
    echo spy selected
    set BINDIR=spy
    set CCFLAGS=-I%IAR_430%\430\inc -I%IAR_430%\430\inc\dlib -Ohz --eec++ --debug -e --double=32 --reduce_stack_usage --dlib_config %IAR_430%\430\lib\dlib\dl430fn.h --diag_suppress Pa050 -DQ_SPY
)

set LIBDIR=%BINDIR%
set LIBFLAGS=
mkdir %BINDIR%

:: QEP ----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qep\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qep.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qfsm_ini.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qfsm_dis.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qhsm_ini.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qhsm_dis.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qhsm_top.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qhsm_in.cpp 

:: QF -----------------------------------------------------------------------
set SRCDIR=..\..\..\..\qf\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_defer.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_fifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_lifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_get_.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_sub.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_usub.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qa_usuba.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qeq_fifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qeq_get.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qeq_init.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qeq_lifo.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_act.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_gc.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_log2.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_new.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_pool.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_psini.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_pspub.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_pwr2.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qf_tick.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qmp_get.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qmp_init.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qmp_put.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qte_ctor.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qte_arm.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qte_darm.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qte_rarm.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qte_ctr.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qvanilla.cpp

:: QS -----------------------------------------------------------------------
if not "%1"=="spy" goto no_spy

set SRCDIR=..\..\..\..\qs\source
set CCINC=-I%QP_PRTDIR% -I%QP_INCDIR% -I%SRCDIR%

%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_blk.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_byte.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_f32.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_f64.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_mem.cpp
%CC% %CCFLAGS% %CCINC% -o%BINDIR% %SRCDIR%\qs_str.cpp

%LIB% %LIBFLAGS% %LIBDIR%\libqp.lib %BINDIR%\qep.r43 %BINDIR%\qfsm_ini.r43 %BINDIR%\qfsm_dis.r43 %BINDIR%\qhsm_ini.r43 %BINDIR%\qhsm_dis.r43 %BINDIR%\qhsm_top.r43 %BINDIR%\qhsm_in.r43 %BINDIR%\qa_defer.r43 %BINDIR%\qa_fifo.r43 %BINDIR%\qa_lifo.r43 %BINDIR%\qa_get_.r43 %BINDIR%\qa_sub.r43 %BINDIR%\qa_usub.r43 %BINDIR%\qa_usuba.r43 %BINDIR%\qeq_fifo.r43 %BINDIR%\qeq_get.r43 %BINDIR%\qeq_init.r43 %BINDIR%\qeq_lifo.r43 %BINDIR%\qf_act.r43 %BINDIR%\qf_gc.r43 %BINDIR%\qf_log2.r43 %BINDIR%\qf_new.r43 %BINDIR%\qf_pool.r43 %BINDIR%\qf_psini.r43 %BINDIR%\qf_pspub.r43 %BINDIR%\qf_pwr2.r43 %BINDIR%\qf_tick.r43 %BINDIR%\qmp_get.r43 %BINDIR%\qmp_init.r43 %BINDIR%\qmp_put.r43 %BINDIR%\qte_ctor.r43 %BINDIR%\qte_arm.r43 %BINDIR%\qte_darm.r43 %BINDIR%\qte_rarm.r43 %BINDIR%\qte_ctr.r43 %BINDIR%\qvanilla.r43 %BINDIR%\qs.r43 %BINDIR%\qs_.r43 %BINDIR%\qs_blk.r43 %BINDIR%\qs_byte.r43 %BINDIR%\qs_f32.r43 %BINDIR%\qs_f64.r43 %BINDIR%\qs_mem.r43 %BINDIR%\qs_str.r43
:: --------------------------------------------------------------------------

:no_spy
%LIB% %LIBFLAGS% %LIBDIR%\libqp.lib %BINDIR%\qep.r43 %BINDIR%\qfsm_ini.r43 %BINDIR%\qfsm_dis.r43 %BINDIR%\qhsm_ini.r43 %BINDIR%\qhsm_dis.r43 %BINDIR%\qhsm_top.r43 %BINDIR%\qhsm_in.r43 %BINDIR%\qa_defer.r43 %BINDIR%\qa_fifo.r43 %BINDIR%\qa_lifo.r43 %BINDIR%\qa_get_.r43 %BINDIR%\qa_sub.r43 %BINDIR%\qa_usub.r43 %BINDIR%\qa_usuba.r43 %BINDIR%\qeq_fifo.r43 %BINDIR%\qeq_get.r43 %BINDIR%\qeq_init.r43 %BINDIR%\qeq_lifo.r43 %BINDIR%\qf_act.r43 %BINDIR%\qf_gc.r43 %BINDIR%\qf_log2.r43 %BINDIR%\qf_new.r43 %BINDIR%\qf_pool.r43 %BINDIR%\qf_psini.r43 %BINDIR%\qf_pspub.r43 %BINDIR%\qf_pwr2.r43 %BINDIR%\qf_tick.r43 %BINDIR%\qmp_get.r43 %BINDIR%\qmp_init.r43 %BINDIR%\qmp_put.r43 %BINDIR%\qte_ctor.r43 %BINDIR%\qte_arm.r43 %BINDIR%\qte_darm.r43 %BINDIR%\qte_rarm.r43 %BINDIR%\qte_ctr.r43 %BINDIR%\qvanilla.r43


:clean
@echo off

erase %BINDIR%\*.r43
rename %BINDIR%\*.lib *.r43

endlocal