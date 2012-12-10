//////////////////////////////////////////////////////////////////////////////
// Product: BSP for YRDKRX62N board, Vanilla, Renesas RX Standard Toolchain
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Oct 18, 2012
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
#include "qp_port.h"
#include "bsp.h"
#include "dpp.h"

#include <machine.h>                                    // intrinsic functions
#include "iodefine.h"
#include "yrdkrx62n.h"

// Q-Spy ---------------------------------------------------------------------
#ifdef Q_SPY

    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;

    uint8_t const QS_CMT0_isr  = 0U;          // identifies event source
    uint8_t const QS_IRQ8_isr  = 0U;          // identifies event source
    uint8_t const QS_IRQ9_isr  = 0U;          // identifies event source
    uint8_t const QS_IRQ12_isr = 0U;          // identifies event source

    #define UART_BAUD_RATE      115200U

    enum AppRecords {                    // application-specific trace records
        PHILO_STAT = QP::QS_USER
    };

#endif

//............................................................................
extern "C" {
#pragma interrupt (CMT0_isr (vect = VECT(CMT0, CMI0)))
void CMT0_isr(void) {
    QF_ISR_ENTRY();     // inform the QF Vanilla kernel about entering the ISR

#ifdef Q_SPY
    QS_tickTime_ += QS_tickPeriod_;          // account for the clock rollover
#endif
    QP::QF::TICK(& QS_CMT0_isr);              // process all armed time events

    QF_ISR_EXIT();       // inform the QF Vanilla kernel about exiting the ISR
}
//............................................................................
#pragma interrupt (IRQ8_isr (vect = VECT(ICU, IRQ8)))
void IRQ8_isr(void) {
    QF_ISR_ENTRY();     // inform the QF Vanilla kernel about entering the ISR

    DPP::AO_Table->POST(Q_NEW(QP::QEvt, DPP::PAUSE_SIG),
                 &QS_IRQ8_isr);
    QF_ISR_EXIT();       // inform the QF Vanilla kernel about exiting the ISR
}
//............................................................................
#pragma interrupt (IRQ9_isr (vect = VECT(ICU, IRQ9)))
void IRQ9_isr(void) {
    QF_ISR_ENTRY();     // inform the QF Vanilla kernel about entering the ISR

    DPP::AO_Philo[0]->POST(Q_NEW(QP::QEvt, DPP::MAX_PUB_SIG),// for testing...
                 &QS_IRQ9_isr);
    QF_ISR_EXIT();       // inform the QF Vanilla kernel about exiting the ISR
}
//............................................................................
#pragma interrupt (IRQ12_isr (vect = VECT(ICU, IRQ12)))
void IRQ12_isr(void) {
    QF_ISR_ENTRY();     // inform the QF Vanilla kernel about entering the ISR

    DPP::AO_Philo[0]->POST(Q_NEW(QP::QEvt, DPP::MAX_PUB_SIG),// for testing...
                 &QS_IRQ12_isr);
    QF_ISR_EXIT();       // inform the QF Vanilla kernel about exiting the ISR
}
//............................................................................
void abort(void) {
    Q_onAssert("abort", 0);
}

}                                                                // extern "C"

//////////////////////////////////////////////////////////////////////////////
namespace DPP {

Q_DEFINE_THIS_FILE

                    // interrupt priorities (higher number is higher priority)
enum InterruptPriorities {
    // lowest urgency
    SW1_INT_PRIORITY  = 3,
    TICK_INT_PRIORITY = 4,
    SW2_INT_PRIORITY  = 5,
    SW3_INT_PRIORITY  = 6,
    // highest urgency
};

// clock configuration -------------------------------------------------------
#define PCLK_FREQ         48000000U
#define TICK_COUNT_VAL    (PCLK_FREQ / 128U / DPP::BSP_TICKS_PER_SEC)

// Local-scope objects -------------------------------------------------------
static uint32_t l_rnd;                                          // random seed

//............................................................................
void BSP_init(void) {
    // init LEDs (GPIOs and LED states to OFF)
    PORTD.DDR.BYTE  = 0xFF;
    PORTE.DDR.BYTE |= 0x0F;
    PORTD.DR.BYTE   = 0xFF;                // initialize all LEDs to OFF state
    PORTE.DR.BYTE  |= 0x0F;                // initialize all LEDs to OFF state

    // Init buttons as GPIO inputs
    // Config GPIO Port 4 as input for reading buttons
    // Not needed after POR because this is the default value...
    //
    PORT4.DDR.BYTE = 0;

    BSP_randomSeed(1234);

    if (QS_INIT((void *)0) == 0) {       // initialize the QS software tracing
        Q_ERROR();
    }
    QS_RESET();
    QS_OBJ_DICTIONARY(&QS_CMT0_isr);
    QS_OBJ_DICTIONARY(&QS_IRQ8_isr);
    QS_OBJ_DICTIONARY(&QS_IRQ9_isr);
    QS_OBJ_DICTIONARY(&QS_IRQ12_isr);
}
//............................................................................
void BSP_terminate(int16_t const result) {
    (void)result;
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
                            // turn LED on when eating and off when not eating
    uint8_t on_off = (stat[0] == 'e' ? LED_ON : LED_OFF);
    switch (n) {
        case 0: LED4 = on_off; break;
        case 1: LED5 = on_off; break;
        case 2: LED6 = on_off; break;
        case 3: LED7 = on_off; break;
        case 4: LED8 = on_off; break;
        default: Q_ERROR();    break;
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n])     // application-specific record begin
        QS_U8(1, n);                                     // Philosopher number
        QS_STR(stat);                                    // Philosopher status
    QS_END()
}
//............................................................................
void BSP_displayPaused(uint8_t const paused) {
    LED11 = (paused != 0U) ? LED_ON : LED_OFF;
}
//............................................................................
uint32_t BSP_random(void) {     // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);
    return l_rnd >> 8;
}
//............................................................................
void BSP_randomSeed(uint32_t const seed) {
    l_rnd = seed;
}

}                                                             // namespace DPP
//////////////////////////////////////////////////////////////////////////////

namespace QP {

//............................................................................
void QF::onStartup(void) {

    // set the interrupt priorities, (manual 11.2.3)
    IPR(CMT0,)     = DPP::TICK_INT_PRIORITY;
    IPR(ICU,IRQ8)  = DPP::SW1_INT_PRIORITY;
    IPR(ICU,IRQ9)  = DPP::SW2_INT_PRIORITY;
    IPR(ICU,IRQ12) = DPP::SW3_INT_PRIORITY;

    // enable GPIO interrupts ...*/
    // configure interrupts for SW1 (IRQ8)
    PORT4.ICR.BIT.B0 = 1U;                     // enable input buffer for P4.0
    ICU.IRQCR[8].BIT.IRQMD = 3U;             // rising & falling edge (11.2.8)
    IEN(ICU, IRQ8) = 1U;                   // enable interrupt source (11.2.2)

    // configure interrupts for SW2 (IRQ9)
    PORT4.ICR.BIT.B1 = 1U;                     // enable input buffer for P4.1
    ICU.IRQCR[9].BIT.IRQMD = 1U;                      // falling edge (11.2.8)
    IEN(ICU, IRQ9) = 1;                    // enable interrupt source (11.2.2)

    // configure interrupts for SW3 (IRQ10)
    PORT4.ICR.BIT.B2 = 1U;                     // enable input buffer for P4.2
    ICU.IRQCR[10].BIT.IRQMD = 1U;                     // falling edge (11.2.8)
    IEN(ICU, IRQ10) = 1;                   // enable interrupt source (11.2.2)

    // configure & start tick timer. Enable CMT0 module. (see manual 9.2.2)
    MSTP(CMT0) = 0U;
    CMT.CMSTR0.BIT.STR0 = 0U;        // Stop CMT0 (channel 0)  (manual 21.2.1)
    CMT0.CMCR.BIT.CKS   = 2U;             // peripheral divider 1/128 (21.2.3)
    CMT0.CMCOR          = TICK_COUNT_VAL;      // compare-match value (21.2.4)
    CMT0.CMCNT          = 0U;           // Clear free-running counter (21.2.4)
    IR(CMT0, CMI0)      = 0U;          // ensure no pending timer ISR (11.2.1)

    IEN(CMT0, CMI0)     = 1U;              // enable interrupt source (11.2.2)
    CMT0.CMCR.BIT.CMIE  = 1U;              // enable timer interrupt  (21.2.3)
    CMT.CMSTR0.BIT.STR0 = 1U;    // start tick timer (CMT0 channel 0) (21.2.1)
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QF::onIdle(void) {             // entered with interrupts DISABLED NOTE01

    LED12 = LED_ON;         // toggle the User LED on and then off, see NOTE02
    LED12 = LED_OFF;

#ifdef Q_SPY
    QF_INT_ENABLE();

    // RX62N has no TX FIFO, just a Transmit Data Register (TDR)
    if (SCI2.SSR.BIT.TEND) {                           // Is SMC's TDR Empty?
        uint16_t b;

        QF_INT_DISABLE();
        b = QS::getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {
            SCI2.TDR = (uint8_t)b;                   // send byte to UART TDR
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular MCU.
    //
    // Sleep mode for RX CPU:
    // Use compiler intrinsic function to put CPU to sleep.
    // Note that we execute this instruction with interrupts disabled,
    // i.e., PSW[I] = 0.
    // HOWEVER, executing the WAIT instruction sets PSW[I] = 1,
    // therefore we do not need to unlock (enable) interrupts upon wakeup,
    // since they will already be enabled.

    wait();
#else
    QF_INT_ENABLE();
#endif
}
//............................................................................
extern "C" void Q_onAssert(char const * const file, int line) {
    (void)file;                                      // avoid compiler warning
    (void)line;                                      // avoid compiler warning
    QF_INT_DISABLE();            // make sure that all interrupts are disabled
    for (;;) {          // NOTE: replace the loop with reset for final version
    }
}

//----------------------------------------------------------------------------
#ifdef Q_SPY
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[6*256];                     // buffer for Quantum Spy
    (void) arg;               // get rid of compiler warning (unused argument)
    initBuf(qsBuf, sizeof(qsBuf));

    // Configure & enable the serial interface used by QS
    // See Figure 28.7 "SCI Initialization Flowchart (Async Mode)"
    MSTP(SCI2) = 0U;                                            // enable SCI2
    SCI2.SCR.BYTE = 0U;                               // disable SCI (TX & RX)
    SCI2.BRR = (PCLK_FREQ / (32U * UART_BAUD_RATE) - 1U);     // set baud rate

    // SCI2 pin select; RXD2 is pin 42 (P5.2), TXD2 is pin 44 (P5.0)
    IOPORT.PFFSCI.BIT.SCI2S = 1U;

    // RXD2 pin = input (set Port 5.2 Input Buffer Control Register)
    PORT5.ICR.BIT.B2 = 1U;

    SCI2.SCR.BIT.CKE = 0U;                      // on-chip baud rate generator
    SCI2.SMR.BYTE = 0U;               // PCLK input; asynchronous; mode: 8,N,1

    // RX interrupt stuff
    IR(SCI2,RXI2)  = 0U;              // clear pending interrupt flag (polled)
    IPR(SCI2,RXI2) = 11U;                        // irq level 11 (pretty high)
    IEN(SCI2,RXI2) = 0U;        // disable RXI2 interrupt (just poll RXI flag)

    // TX interrupt stuff
    IR(SCI2,TXI2)  = 0U;              // clear pending interrupt flag (polled)
    IPR(SCI2,TXI2) = 10U;                        // irq level 11 (pretty high)
    IEN(SCI2,TXI2) = 0U;        // disable TXI2 interrupt (just poll RXI flag)

    SCI2.SCR.BYTE |= 0x30U;                 // enable RX & TX at the same time

    // Initialize QS timing variables
    QS_tickPeriod_ = TICK_COUNT_VAL;
    QS_tickTime_   = 0;                       // count up timer starts at zero

                                                    // setup the QS filters...
    QS_FILTER_ON(QS_ALL_RECORDS);

//    QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
//    QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
//    QS_FILTER_OFF(QS_QEP_STATE_EXIT);
//    QS_FILTER_OFF(QS_QEP_STATE_INIT);
//    QS_FILTER_OFF(QS_QEP_INIT_TRAN);
//    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
//    QS_FILTER_OFF(QS_QEP_TRAN);
//    QS_FILTER_OFF(QS_QEP_IGNORED);

    QS_FILTER_OFF(QS_QF_ACTIVE_ADD);
    QS_FILTER_OFF(QS_QF_ACTIVE_REMOVE);
    QS_FILTER_OFF(QS_QF_ACTIVE_SUBSCRIBE);
//    QS_FILTER_OFF(QS_QF_ACTIVE_UNSUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
//    QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET_LAST);
    QS_FILTER_OFF(QS_QF_EQUEUE_INIT);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET_LAST);
    QS_FILTER_OFF(QS_QF_MPOOL_INIT);
    QS_FILTER_OFF(QS_QF_MPOOL_GET);
    QS_FILTER_OFF(QS_QF_MPOOL_PUT);
    QS_FILTER_OFF(QS_QF_PUBLISH);
    QS_FILTER_OFF(QS_QF_NEW);
    QS_FILTER_OFF(QS_QF_GC_ATTEMPT);
    QS_FILTER_OFF(QS_QF_GC);
//    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QF_TIMEEVT_ARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_AUTO_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM_ATTEMPT);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_REARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_POST);
    QS_FILTER_OFF(QS_QF_CRIT_ENTRY);
    QS_FILTER_OFF(QS_QF_CRIT_EXIT);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return true;                                             // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {              // invoked with interrupts locked
    if (IR(CMT0, CMI0)) {                              // is tick ISR pending?
        return QS_tickTime_ + CMT0.CMCNT + QS_tickPeriod_;
    }
    else {                                              // no rollover occured
        return QS_tickTime_ + CMT0.CMCNT;
    }
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;
    b = getByte();
    while (b != QS_EOD) {    // next QS trace byte available?
        while (SCI2.SSR.BIT.TEND == 0) {                      // TX not ready?
        }
        SCI2.TDR = (uint8_t)b;                        // send byte to UART TDR

        b = getByte();                                 // try to get next byte
    }
}
#endif                                                                // Q_SPY

}                                                              // namespace QP

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// The QF::onIdle() callback is called with interrupts disabled, because the
// determination of the idle condition might change by any interrupt posting
// an event. QF::onIdle() must internally enable interrupts, ideally
// atomically with putting the CPU to the power-saving mode.
//
// NOTE02:
// The LED12 is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//


