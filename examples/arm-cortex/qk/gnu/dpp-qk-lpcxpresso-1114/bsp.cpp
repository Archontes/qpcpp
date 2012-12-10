//////////////////////////////////////////////////////////////////////////////
// Product: DPP example, LPCXpresso-1114 board, QK kernel
// Last Updated for Version: 4.5.02
// Date of the Last Update:  Oct 05, 2012
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
#include "dpp.h"
#include "bsp.h"

extern "C" {
    #include "LPC11xx.h"                                // LPC11xx definitions
    #include "timer16.h"
    #include "clkconfig.h"
    #include "gpio.h"
#ifdef Q_SPY
    #include "uart.h"
#endif
}

//////////////////////////////////////////////////////////////////////////////
namespace DPP {

Q_DEFINE_THIS_FILE

#define LED_PORT    0
#define LED_BIT     7
#define LED_ON      1
#define LED_OFF     0

enum ISR_Priorities {      // ISR priorities starting from the highest urgency
    PIOINT0_PRIO,
    SYSTICK_PRIO,
    // ...
};

//............................................................................
static unsigned  l_rnd;                                         // random seed

#ifdef Q_SPY

    QP::QSTimeCtr QS_tickTime_;
    QP::QSTimeCtr QS_tickPeriod_;
    static uint8_t l_SysTick_Handler;
    static uint8_t l_GPIOPortA_IRQHandler;

    #define QS_BUF_SIZE   (2*1024)
    #define QS_BAUD_RATE  115200

    enum AppRecords {                    // application-specific trace records
        PHILO_STAT = QP::QS_USER
    };
#endif

//............................................................................
extern "C" void SysTick_Handler(void) __attribute__((__interrupt__));
extern "C" void SysTick_Handler(void) {
    QK_ISR_ENTRY();                          // inform QK-nano about ISR entry
#ifdef Q_SPY
    uint32_t dummy = SysTick->CTRL;           // clear NVIC_ST_CTRL_COUNT flag
    QS_tickTime_ += QS_tickPeriod_;          // account for the clock rollover
#endif

    QP::QF::TICK(&l_SysTick_Handler);         // process all armed time events

    QK_ISR_EXIT();                            // inform QK-nano about ISR exit
}
//............................................................................
extern "C" void PIOINT0_IRQHandler(void) __attribute__((__interrupt__));
extern "C" void PIOINT0_IRQHandler(void) {
    QK_ISR_ENTRY();                          // inform QK-nano about ISR entry

    AO_Table->POST(Q_NEW(QP::QEvt, MAX_PUB_SIG),                // for testing
                   &l_GPIOPortA_IRQHandler);

    QK_ISR_EXIT();                            // inform QK-nano about ISR exit
}

//............................................................................
void BSP_init(void) {
    SystemInit();                            // initialize the clocking system

    GPIOInit();                             // initialize GPIO (sets up clock)
    GPIOSetDir(LED_PORT, LED_BIT, 1);            // set port for LED to output

    if (QS_INIT((void *)0) == 0) {       // initialize the QS software tracing
        Q_ERROR();
    }
    QS_RESET();
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_GPIOPortA_IRQHandler);
}
//............................................................................
void BSP_terminate(int16_t result) {
    (void)result;
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == 'e') {
        GPIOSetValue(LED_PORT, LED_BIT, LED_ON);                     // LED on
    }
    else {
        GPIOSetValue(LED_PORT, LED_BIT, LED_OFF);                   // LED off
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n])     // application-specific record begin
        QS_U8(1, n);                                     // Philosopher number
        QS_STR(stat);                                    // Philosopher status
    QS_END()
}
//............................................................................
void BSP_displayPaused(uint8_t paused) {
    (void)paused;
}
//............................................................................
uint32_t BSP_random(void) {     // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * (3*7*11*13*23);
    return l_rnd >> 8;
}
//............................................................................
void BSP_randomSeed(uint32_t seed) {
    l_rnd = seed;
}

}                                                             // namespace DPP
//////////////////////////////////////////////////////////////////////////////

//............................................................................
extern "C" void Q_onAssert(char const * const file, int line) {
    (void)file;                                      // avoid compiler warning
    (void)line;                                      // avoid compiler warning
    QF_INT_DISABLE();            // make sure that all interrupts are disabled
    for (;;) {          // NOTE: replace the loop with reset for final version
    }
}
//............................................................................
// error routine that is called if the STM32 library encounters an error
extern "C" void assert_failed(char const *file, int line) {
    Q_onAssert(file, line);
}

namespace QP {

//............................................................................
void QF::onStartup(void) {
    // Set up and enable the SysTick timer. It will be used as a reference
    // for delay loops in the interrupt handlers. The SysTick timer period
    // will be set up for BSP_TICKS_PER_SEC.
    //
    SysTick_Config(SystemCoreClock / DPP::BSP_TICKS_PER_SEC);

              // enable EINT0 interrupt, which is used for testing preemptions
    NVIC_EnableIRQ(EINT0_IRQn);

                          // set priorities of all interrupts in the system...
    NVIC_SetPriority(SysTick_IRQn, DPP::SYSTICK_PRIO);
    NVIC_SetPriority(EINT0_IRQn,   DPP::PIOINT0_PRIO);
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QK::onIdle(void) {
                            // toggle the blue LED on and then off, see NOTE01
    //QF_INT_DISABLE();
    //GPIOSetValue(LED_PORT, LED_BIT, LED_ON);                        // LED on
    //GPIOSetValue(LED_PORT, LED_BIT, LED_OFF);                      // LED off
    //QF_INT_ENABLE();

#ifdef Q_SPY

    if ((LPC_UART->LSR & LSR_THRE) != 0) {                    // is THR empty?

        QF_INT_DISABLE();
        uint16_t b = QS::getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {                                 // not End-Of-Data?
           LPC_UART->THR = (b & 0xFF);            // put into the THR register
        }
    }

#elif defined NDEBUG
    // put the CPU and peripherals to the low-power mode
    __WFI();                   // stop clocking the CPU and wait for interrupt
#endif
}

//----------------------------------------------------------------------------
#ifdef Q_SPY
//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[QS_BUF_SIZE];               // buffer for Quantum Spy
    initBuf(qsBuf, sizeof(qsBuf));

    UARTInit(QS_BAUD_RATE);  // initialize the UART with the desired baud rate
    NVIC_DisableIRQ(UART_IRQn); // do not use the interrupts (QS uses polling)
    LPC_UART->IER = 0;

    DPP::QS_tickPeriod_ = (QSTimeCtr)(SystemCoreClock/DPP::BSP_TICKS_PER_SEC);
    DPP::QS_tickTime_ = DPP::QS_tickPeriod_; // to start the timestamp at zero

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
    QS_FILTER_OFF(QS_QF_ACTIVE_UNSUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
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

//    QS_FILTER_OFF(QS_QK_MUTEX_LOCK);
//    QS_FILTER_OFF(QS_QK_MUTEX_UNLOCK);
    QS_FILTER_OFF(QS_QK_SCHEDULE);

    return true;                                             // return success
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {              // invoked with interrupts locked
    if ((SysTick->CTRL & 0x00010000U) == 0U) {                // COUNT no set?
        return DPP::QS_tickTime_ - (QSTimeCtr)SysTick->VAL;
    }
    else {        // the rollover occured, but the SysTick_ISR did not run yet
        return DPP::QS_tickTime_ - (QSTimeCtr)SysTick->VAL
               + DPP::QS_tickPeriod_;
    }
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;
    while ((b = getByte()) != QS_EOD) {            // while not End-Of-Data...
        while ((LPC_UART->LSR & LSR_THRE) == 0) {       // while TXE not empty
        }
        LPC_UART->THR = (b & 0xFF);               // put into the THR register
    }
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------

}                                                              // namespace QP

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
