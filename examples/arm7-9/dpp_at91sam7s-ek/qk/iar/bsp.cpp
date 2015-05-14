//****************************************************************************
// Product: DPP on AT91SAM7S-EK, preemptive QK kernel, IAR-ARM toolset
// Last Updated for Version: 5.4.0
// Date of the Last Update:  2015-05-04
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
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
// Web  : http://www.state-machine.com
// Email: info@state-machine.com
//****************************************************************************
#include "qpcpp.h"
#include "dpp.h"
#include "bsp.h"

#include "AT91SAM7S64.h"  // Atmel AT91SAM7S64 MCU

#pragma diag_suppress=Ta021  // call __iar_disable_interrupt from __ramfunc
#pragma diag_suppress=Ta022  // possible ROM access <ptr> from __ramfunc
#pragma diag_suppress=Ta023  // call to non __ramfunc from __ramfunc

// extern "C" functions in C =================================================
extern "C" {
//............................................................................
__ramfunc
void BSP_irq(void) {
    IntVector vect = (IntVector)AT91C_BASE_AIC->AIC_IVR; // read the IVR
    AT91C_BASE_AIC->AIC_IVR = (AT91_REG)vect; // write AIC_IVR if protected

    QF_INT_ENABLE();    // allow nesting interrupts
    (*vect)();          // call the IRQ ISR via the pointer to function
    QF_INT_DISABLE();   // disable interrups for the exit sequence

    AT91C_BASE_AIC->AIC_EOICR = 0; // write AIC_EOICR to clear interrupt
}

} // extern "C"

namespace DPP {

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
typedef void (*IntVector)(void);  // IntVector pointer-to-function

uint32_t const l_led[] = {
    (1U << 0),              // LED D1 on AT91SAM7S-EK
    (1U << 1),              // LED D2 on AT91SAM7S-EK
    (1U << 2),              // LED D3 on AT91SAM7S-EK
    (1U << 3),              // LED D4 on AT91SAM7S-EK
    0U                      // no LED 5  on AT91SAM7S-EK
};

#define LED_ON(num_)       (AT91C_BASE_PIOA->PIO_CODR = l_led[num_])
#define LED_OFF(num_)      (AT91C_BASE_PIOA->PIO_SODR = l_led[num_])

static unsigned  l_rnd;    // random seed

#ifdef Q_SPY

    static uint8_t const l_ISR_tick = 0;
    enum AppRecords {      // application-specific trace records
      PHILO_STAT = QP::QS_USER
    };

#endif

// ISRs ======================================================================
__ramfunc
static void ISR_tick(void) {
    uint32_t volatile tmp = AT91C_BASE_PITC->PITC_PIVR;  // clear interrupt
    (void)tmp; // avoid the compiler warning about unused variable

    QP::QF::TICK_X(0U, &l_ISR_tick); // process all time events at tick rate 0
}
//............................................................................
__ramfunc
static void ISR_spur(void) {
}


// BSP functions =============================================================
void BSP_init(void) {
    uint32_t i;

    for (i = 0; i < Q_DIM(l_led); ++i) {      // initialize the LEDs...
        AT91C_BASE_PIOA->PIO_PER = l_led[i];  // enable pin
        AT91C_BASE_PIOA->PIO_OER = l_led[i];  // configure as output pin
        LED_OFF(i);                           // extinguish the LED
    }

    // configure Advanced Interrupt Controller (AIC) of AT91...
    AT91C_BASE_AIC->AIC_IDCR = ~0;            // disable all interrupts
    AT91C_BASE_AIC->AIC_ICCR = ~0;            // clear all interrupts
    for (i = 0; i < 8; ++i) {
        AT91C_BASE_AIC->AIC_EOICR = 0;        // write AIC_EOICR 8 times
    }

    // set the desired ticking rate for the PIT...
    i = (get_MCK_FREQ() / 16U / BSP_TICKS_PER_SEC) - 1U;
    AT91C_BASE_PITC->PITC_PIMR = (AT91C_PITC_PITEN | AT91C_PITC_PITIEN | i);

    BSP_randomSeed(1234U); // seed the random number generator

    if (QS_INIT((void *)0) == 0) { // initialize the QS software tracing
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_ISR_tick);
}
//............................................................................
void BSP_terminate(int16_t result) {
    (void)result;
}
//............................................................................
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == (uint8_t)'e') { // is this Philosopher eating?
        LED_ON(n);
    }
    else {  // this Philosopher is not eating
        LED_OFF(n);
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n]) // application-specific record begin
        QS_U8(1, n);                  // Philosopher number
        QS_STR(stat);                 // Philosopher status
    QS_END()
}
//............................................................................
void BSP_displayPaused(uint8_t paused) {
    (void)paused;
}
//............................................................................
uint32_t BSP_random(void) {  // a very cheap pseudo-random-number generator
    // "Super-Duper" Linear Congruential Generator (LCG)
    // LCG(2^32, 3*7*11*13*23, 0, seed)
    //
    l_rnd = l_rnd * (3U*7U*11U*13U*23U);
    return l_rnd >> 8;
}
//............................................................................
void BSP_randomSeed(uint32_t seed) {
    l_rnd = seed;
}

} // namespace DPP


namespace QP {

// QF callbacks ==============================================================
void QF::onStartup(void) {
    // hook the exception handlers from the QF port...
    *(uint32_t volatile *)0x24 = (uint32_t)&QF_undef;
    *(uint32_t volatile *)0x28 = (uint32_t)&QF_swi;
    *(uint32_t volatile *)0x2C = (uint32_t)&QF_pAbort;
    *(uint32_t volatile *)0x30 = (uint32_t)&QF_dAbort;
    *(uint32_t volatile *)0x34 = (uint32_t)&QF_reserved;
    *(uint32_t volatile *)0x38 = (uint32_t)&QK_irq;
    *(uint32_t volatile *)0x3C = (uint32_t)0; // unimplemented!

    AT91C_BASE_AIC->AIC_SVR[AT91C_ID_SYS] = (uint32_t)&DPP::ISR_tick;
    AT91C_BASE_AIC->AIC_SPU = (uint32_t)&DPP::ISR_spur; // spurious IRQ

    AT91C_BASE_AIC->AIC_SMR[AT91C_ID_SYS] =
        (AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL | AT91C_AIC_PRIOR_LOWEST);
    AT91C_BASE_AIC->AIC_ICCR = (1 << AT91C_ID_SYS);
    AT91C_BASE_AIC->AIC_IECR = (1 << AT91C_ID_SYS);
}
//............................................................................
void QF_onCleanup(void) {
}
//............................................................................
__ramfunc
void QK::onIdle(void) {
#ifdef Q_SPY
    // use the idle cycles for QS transmission...

    if ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXBUFE) != 0) { // not busy?
        uint16_t nBytes = 0xFFFFU; // get all available bytes
        uint8_t const *block;

        QF_INT_DISABLE();
        if ((block = QS::getBlock(&nBytes)) != (uint8_t *)0) { // new block?
            AT91C_BASE_DBGU->DBGU_TPR = (uint32_t)block;
            AT91C_BASE_DBGU->DBGU_TCR = (uint32_t)nBytes;
            nBytes = 0xFFFFU; // get all available bytes
            if ((block = QS::getBlock(&nBytes)) != (uint8_t *)0) {//another?
                AT91C_BASE_DBGU->DBGU_TNPR = (uint32_t)block;
                AT91C_BASE_DBGU->DBGU_TNCR = (uint32_t)nBytes;
            }
        }
        QF_INT_ENABLE();
    }
#elif defined NDEBUG // only if not debugging (idle mode hinders debugging)
    AT91C_BASE_PMC->PMC_SCDR = 1;// Power-Management: disable the CPU clock
    // NOTE: an interrupt starts the CPU clock again
#endif
}
//............................................................................
extern "C" void Q_onAssert(char const * const file, int line) {
    QF_INT_DISABLE(); // disable all interrupts
    for (;;) {        // hang here in the for-ever loop
    }
}

// QS callbacks ==============================================================
#ifdef Q_SPY
uint32_t l_timeOverflow;

#define QS_BUF_SIZE        (2*1024)
#define BAUD_RATE          115200U

bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[QS_BUF_SIZE]; // buffer for Quantum Spy
    AT91PS_DBGU pDBGU = AT91C_BASE_DBGU;
    AT91PS_TC   pTC0  = AT91C_BASE_TC0;// TC0 used for timestamp generation
    uint32_t volatile tmp;

    initBuf(qsBuf, sizeof(qsBuf));

    // configure the Debug UART for QSPY output ...
    AT91C_BASE_PIOA->PIO_PDR = AT91C_PA10_DTXD; // configure pin as DTXD

    pDBGU->DBGU_CR   = AT91C_US_TXEN;   // enable only transmitter
    pDBGU->DBGU_IDR  = ~0U;              // disable all DBGU interrupts
    pDBGU->DBGU_MR   = AT91C_US_PAR_NONE;  // no parity bit
    pDBGU->DBGU_BRGR = ((get_MCK_FREQ()/BAUD_RATE + 8) >> 4); // baud rate
    pDBGU->DBGU_PTCR = AT91C_PDC_TXTEN; // enable PDC transfer from DBGU

    // configure Timer/Counter 0 for time measurements ...
    AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_TC0); // enable clock to TC0

    pTC0->TC_CCR = AT91C_TC_CLKDIS; // TC_CCR: disable Clock Counter
    pTC0->TC_IDR = ~0;              // TC_IDR: disable all timer interrupts
    tmp = pTC0->TC_SR;              // TC_SR: read
    (void)tmp; // avoid the compiler warning about the unused variable

    // CPCTRG, MCK/32 clock...
    pTC0->TC_CMR = (AT91C_TC_CPCTRG | AT91C_TC_CLKS_TIMER_DIV3_CLOCK);
    pTC0->TC_CCR = AT91C_TC_CLKEN;  // TC_CCR: enable Clock Counter
    pTC0->TC_CCR = AT91C_TC_SWTRG;  // TC_CCR: start counting

    // setup the QS filters...
    QS_FILTER_ON(QS_QEP_STATE_ENTRY);
    QS_FILTER_ON(QS_QEP_STATE_EXIT);
    QS_FILTER_ON(QS_QEP_STATE_INIT);
    QS_FILTER_ON(QS_QEP_INIT_TRAN);
    QS_FILTER_ON(QS_QEP_INTERN_TRAN);
    QS_FILTER_ON(QS_QEP_TRAN);
    QS_FILTER_ON(QS_QEP_IGNORED);
    QS_FILTER_ON(QS_QEP_DISPATCH);
    QS_FILTER_ON(QS_QEP_UNHANDLED);

//    QS_FILTER_ON(QS_QF_ACTIVE_ADD);
//    QS_FILTER_ON(QS_QF_ACTIVE_REMOVE);
//    QS_FILTER_ON(QS_QF_ACTIVE_SUBSCRIBE);
//    QS_FILTER_ON(QS_QF_ACTIVE_UNSUBSCRIBE);
//    QS_FILTER_ON(QS_QF_ACTIVE_POST_FIFO);
//    QS_FILTER_ON(QS_QF_ACTIVE_POST_LIFO);
//    QS_FILTER_ON(QS_QF_ACTIVE_GET);
//    QS_FILTER_ON(QS_QF_ACTIVE_GET_LAST);
//    QS_FILTER_ON(QS_QF_EQUEUE_INIT);
//    QS_FILTER_ON(QS_QF_EQUEUE_POST_FIFO);
//    QS_FILTER_ON(QS_QF_EQUEUE_POST_LIFO);
//    QS_FILTER_ON(QS_QF_EQUEUE_GET);
//    QS_FILTER_ON(QS_QF_EQUEUE_GET_LAST);
//    QS_FILTER_ON(QS_QF_MPOOL_INIT);
//    QS_FILTER_ON(QS_QF_MPOOL_GET);
//    QS_FILTER_ON(QS_QF_MPOOL_PUT);
//    QS_FILTER_ON(QS_QF_PUBLISH);
//    QS_FILTER_ON(QS_QF_RESERVED8);
//    QS_FILTER_ON(QS_QF_NEW);
//    QS_FILTER_ON(QS_QF_GC_ATTEMPT);
//    QS_FILTER_ON(QS_QF_GC);
    QS_FILTER_ON(QS_QF_TICK);
//    QS_FILTER_ON(QS_QF_TIMEEVT_ARM);
//    QS_FILTER_ON(QS_QF_TIMEEVT_AUTO_DISARM);
//    QS_FILTER_ON(QS_QF_TIMEEVT_DISARM_ATTEMPT);
//    QS_FILTER_ON(QS_QF_TIMEEVT_DISARM);
//    QS_FILTER_ON(QS_QF_TIMEEVT_REARM);
//    QS_FILTER_ON(QS_QF_TIMEEVT_POST);
//    QS_FILTER_ON(QS_QF_TIMEEVT_CTR);
//    QS_FILTER_ON(QS_QF_CRIT_ENTRY);
//    QS_FILTER_ON(QS_QF_CRIT_EXIT);
//    QS_FILTER_ON(QS_QF_ISR_ENTRY);
//    QS_FILTER_ON(QS_QF_ISR_EXIT);
//    QS_FILTER_ON(QS_QF_INT_DISABLE);
//    QS_FILTER_ON(QS_QF_INT_ENABLE);
//    QS_FILTER_ON(QS_QF_ACTIVE_POST_ATTEMPT);
//    QS_FILTER_ON(QS_QF_EQUEUE_POST_ATTEMPT);
//    QS_FILTER_ON(QS_QF_MPOOL_GET_ATTEMPT);
//    QS_FILTER_ON(QS_QF_RESERVED1);
//    QS_FILTER_ON(QS_QF_RESERVED0);

//    QS_FILTER_ON(QS_QK_MUTEX_LOCK);
//    QS_FILTER_ON(QS_QK_MUTEX_UNLOCK);
//    QS_FILTER_ON(QS_QK_SCHEDULE);
//    QS_FILTER_ON(QS_QK_RESERVED1);
//    QS_FILTER_ON(QS_QK_RESERVED0);

//    QS_FILTER_ON(QS_QEP_TRAN_HIST);
//    QS_FILTER_ON(QS_QEP_TRAN_EP);
//    QS_FILTER_ON(QS_QEP_TRAN_XP);
//    QS_FILTER_ON(QS_QEP_RESERVED1);
//    QS_FILTER_ON(QS_QEP_RESERVED0);

    QS_FILTER_ON(QS_SIG_DICT);
    QS_FILTER_ON(QS_OBJ_DICT);
    QS_FILTER_ON(QS_FUN_DICT);
    QS_FILTER_ON(QS_USR_DICT);
    QS_FILTER_ON(QS_EMPTY);
    QS_FILTER_ON(QS_RESERVED3);
    QS_FILTER_ON(QS_RESERVED2);
    QS_FILTER_ON(QS_TEST_RUN);
    QS_FILTER_ON(QS_TEST_FAIL);
    QS_FILTER_ON(QS_ASSERT_FAIL);

    return true; // indicate successfull QS initialization
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
void QS::onFlush(void) {
    uint16_t nBytes = 0xFFFFU; // get all available bytes
    uint8_t const *block;
    while ((AT91C_BASE_DBGU->DBGU_CSR & AT91C_US_TXBUFE) == 0) { // busy?
    }
    if ((block = getBlock(&nBytes)) != (uint8_t *)0) {
        AT91C_BASE_DBGU->DBGU_TPR = (uint32_t)block;
        AT91C_BASE_DBGU->DBGU_TCR = (uint32_t)nBytes;
        nBytes = 0xFFFFU; // get all available bytes
        if ((block = getBlock(&nBytes)) != (uint8_t *)0) {
            AT91C_BASE_DBGU->DBGU_TNPR = (uint32_t)block;
            AT91C_BASE_DBGU->DBGU_TNCR = (uint32_t)nBytes;
        }
    }
}
//............................................................................
// NOTE: getTime is invoked within a critical section (inetrrupts disabled)
__ramfunc
uint32_t QS::onGetTime(void) {
    AT91PS_TC pTC0  = AT91C_BASE_TC0;  // TC0 used for timestamp generation
    uint32_t now = pTC0->TC_CV; // get the counter value
                                // did the timer overflow 0xFFFF?
    if ((pTC0->TC_SR & AT91C_TC_COVFS) != 0) {
        l_timeOverflow += (uint32_t)0x10000; // account for the overflow
    }
    return l_timeOverflow + now;
}
#endif  // Q_SPY
//----------------------------------------------------------------------------

} // namespace QP