//////////////////////////////////////////////////////////////////////////////
// Product: Board Support Package for STK600-ATmega128A1
// Last Updated for Version: 4.5.00
// Date of the Last Update:  May 20, 2012
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

Q_DEFINE_THIS_FILE

#define LED_ON(num_)       (PORTD.OUTCLR = (1 << (num_)))
#define LED_OFF(num_)      (PORTD.OUTSET = (1 << (num_)))
#define LED_TOGGLE(num_)   (PORTD.OUTTGL = (1 << (num_)))
#define LED_ON_ALL()       (PORTD.OUTCLR = 0xFF)
#define LED_OFF_ALL()      (PORTD.OUTSET = 0xFF)

// Local objects -------------------------------------------------------------
#ifdef Q_SPY
    #define QS_BUF_SIZE    (512)
    #define BAUD_RATE      38400

    static uint32_t l_tickTime;

    enum AppRecords {                    // application-specific trace records
        PHILO_STAT = QS_USER
    };
#endif

// ISRs ----------------------------------------------------------------------
#pragma vector = TCC0_OVF_vect
QK_ISR(TCC0_OVF) {                         // interrupt service routine for QK

    // No need to clear the interrupt source since the Timer0 compare
    // interrupt is automatically cleard in hardware when the ISR runs.

#ifdef Q_SPY
    l_tickTime += (F_CPU / BSP_TICKS_PER_SEC / 256);
#endif

    QF::tick();
}
//............................................................................
#pragma vector = TCC0_ERR_vect
QK_ISR(TCC0_ERR) {                         // interrupt service routine for QK
    static QEvt const testevt = { EAT_SIG, 0 };
                        // post an event to the highest-priority active object
    AO_Table->postFIFO(&testevt);
}

//............................................................................
void BSP_init(void) {
    OSC.CTRL |= OSC_RC32MEN_bm;                  // enable internal 32MHz osc.
    while ((OSC.STATUS & OSC_RC32MEN_bm) == 0) {      // wait until osc. ready
    }
    CCP = CCP_IOREG_gc;      // enable change in the Configuration Change Reg.
    CLK.CTRL = CLK_SCLKSEL_RC32M_gc;      // select internal 32MHz osc. source

    PORTD.DIRSET = 0xFF;                // all PORTD pins are outputs for LEDs
    LED_OFF_ALL();                                        // trun off all LEDs

                                        // enable interrupt levels 1-3 in PMIC
    PMIC.CTRL |= (1 << 0) | (1 << 1) | (1 << 2);

    if (QS_INIT((void *)0) == 0) {       // initialize the QS software tracing
        Q_ERROR();
    }
}
//............................................................................
void QF::onStartup(void) {
    __disable_interrupt();       // make sure that all interrupts are disabled

    TCC0.CTRLA = 0x06;                    // set TIMER C0 prescaler to CLK/256
    TCC0.CTRLB = 0x00;
    TCC0.CTRLC = 0x00;
    TCC0.CTRLD = 0x00;
                                                  // load 16-bit TIMER0 period
    TCC0.PER   = (uint16_t)((F_CPU + (256 * BSP_TICKS_PER_SEC / 2))
                              / ((256 * BSP_TICKS_PER_SEC)));

    TCC0.INTCTRLA = 0x01;    // enable overflow interrupt on TIMER0 at level 1
    //TCC0.INTCTRLA = 0x02;    // enable overflow interrupt on TIMER0 at level 2
    //TCC0.INTCTRLA = 0x03;    // enable overflow interrupt on TIMER0 at level 3

    TCC0.INTCTRLA |= (0x03 << 2);    // enable error int. on TIMER0 at level 3

    TCC0.CTRLFSET = (1 << 2);                              // update the timer
    TCC0.CTRLFCLR = (1 << 2);

    TCC0.CTRLFSET = (1 << 3);                             // restart the timer
    TCC0.CTRLFCLR = (1 << 3);

    __enable_interrupt();         // make sure that all interrupts are enabled
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void QK_init(void) {
}
//............................................................................
void QK::onIdle(void) {
                        // toggle the LED number 7 on and then off, see NOTE01
    QF_INT_LOCK(dummy);
    LED_ON(7);
    LED_OFF(7);
    QF_INT_UNLOCK(dummy);

#ifdef Q_SPY                        // use the idle cycles for QS transmission

    QF_INT_LOCK(dummy);                                     // lock interrupts
    if ((USARTC0.STATUS & USART_DREIF_bm) != 0) {          // TX buffer empty?
        uint16_t b = QS::getByte();
        if (b != QS_EOD) {
            USARTC0.DATA = (uint8_t)b;    // stick the byte to the TX data reg
        }
    }
    QF_INT_UNLOCK(dummy);                                 // unlock interrupts

#elif defined NDEBUG
                        // configure idle sleep mode, , adjust to your project
    SLEEP.CTRL = SLEEP_SMODE_IDLE_gc | SLEEP_SEN_bm;
    __sleep();
#endif                                                                // Q_SPY
}
//............................................................................
void BSP_displyPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == (uint8_t)'e') {              // is this Philosopher eating?
        LED_ON(n);
    }
    else {                                   // this Philosopher is not eating
        LED_OFF(n);
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n])     // application-specific record begin
        QS_U8(1, n);                                     // Philosopher number
        QS_STR(stat);                                    // Philosopher status
    QS_END()
}
//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    __disable_interrupt();                              // lock all interrupts
    LED_ON_ALL();                                               // all LEDs on
    for (;;) {                               // hang here in the for-ever loop
    }
}
//............................................................................
// The WinAVR/avr-g++ compiler generates somehow a reference to the operator
// delete for every class with a virtual destructor. This project does NOT
// use new or delete, but to satisfy the linker the following dummy
// implementation of the operator delete will cause ERROR when it is used.
void operator delete(void *obj) {
    Q_ERROR();
}

//----------------------------------------------------------------------------
#ifdef Q_SPY
uint8_t QS::onStartup(void const *arg) {
    static uint8_t qsBuf[QS_BUF_SIZE];               // buffer for Quantum Spy
    uint16_t n;

    initBuf(qsBuf, sizeof(qsBuf));

    // The following PORT setting is only valid to USARTC0.
    // For other USARTs, you might need to use different PORT and/or pins.
    //
    PORTC.DIRSET = PIN3_bm;                           // PIN3 (TXD0) as output
    PORTC.DIRCLR = PIN2_bm;                             // PC2 (RXD0) as input


    n = F_CPU / 16 / BAUD_RATE - 1;                           // Set baud rate
    USARTC0.BAUDCTRLA = (uint8_t)n;
    USARTC0.BAUDCTRLB = (uint8_t)(n >> 8);

                           // enable transmitter in polled mode, no interrupts
    USARTC0.CTRLB |= USART_TXEN_bm;

                               // USARTC0, 8 Data bits, No Parity, 1 Stop bit.
    USARTC0.CTRLC = (uint8_t)(USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc);

                                                    // setup the QS filters...
    QS_FILTER_ON(QS_ALL_RECORDS);

//    QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
//    QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
//    QS_FILTER_OFF(QS_QEP_STATE_EXIT);
//    QS_FILTER_OFF(QS_QEP_STATE_INIT);
//    QS_FILTER_OFF(QS_QEP_INIT_TRAN);
//    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
//    QS_FILTER_OFF(QS_QEP_TRAN);
//    QS_FILTER_OFF(QS_QEP_dummyD);

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
    QS_FILTER_OFF(QS_QF_INT_LOCK);
    QS_FILTER_OFF(QS_QF_INT_UNLOCK);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return (uint8_t)1;               // indicate successfull QS initialization
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;
    while ((b = getByte()) != QS_EOD) {       // next QS trace byte available?
                                             // wait while TX buffer not empty
        while ((USARTC0.STATUS & USART_DREIF_bm) == 0) {
        }
        USARTC0.DATA = (uint8_t)b;        // stick the byte to the TX data reg
    }
}
//............................................................................
// NOTE: invoked within a critical section (inetrrupts disabled)
QSTimeCtr QS::onGetTime(void) {
    uint8_t cntl = TCC0.CNTL;   // read the low-byte first to ensure atomicity
    QSTimeCtr cnt = ((QSTimeCtr)TCC0.CNTH << 8) | cntl;

    if ((TCC0.INTFLAGS & TC0_CCAIF_bm) == 0) {      // output compare NOT set?
        return l_tickTime + cnt;
    }
    else {          // the output compare occured, but the ISR did not run yet
        return l_tickTime + (F_CPU / BSP_TICKS_PER_SEC / 256) + cnt;
    }
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// The LED[7] is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of invcations of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
