//////////////////////////////////////////////////////////////////////////////
// Product: DPP example, QK preemptive kernel
// Last Updated for Version: 4.5.03
// Date of the Last Update:  Feb 01, 2013
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
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

#include "system.h"
#include "altera_avalon_pio_regs.h"

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
uint8_t l_led;

#define LED_ON(num_)   do { \
    l_led |= (1 << (num_)); \
    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, l_led); \
} while (0)
#define LED_OFF(num_)      do { \
    l_led &= ~(1 << (num_)); \
    IOWR_ALTERA_AVALON_PIO_DATA(LED_PIO_BASE, l_led); \
} while (0)


#ifdef Q_SPY

    //#include "altera_avalon_uart_regs.h"               // uncomment for UART
    #include "altera_avalon_jtag_uart_regs.h"     // uncomment for JTAG UART
    #include "altera_avalon_timer_regs.h"

    #define QS_BUF_SIZE        (1024*4)

    enum AppRecords {                    // application-specific trace records
        PHILO_STAT = QS_USER
    };
#endif                                                                // Q_SPY

//............................................................................
void BSP_init(void) {

    // low-level hardware initialization already done in the Altera HAL

    if (QS_INIT((void *)0) == 0) {       // initialize the QS software tracing
        Q_ERROR();
    }
}
//............................................................................
void QK_init(void) {
}
//............................................................................
void QK::onIdle(void) {
#ifdef Q_SPY                        // use the idle cycles for QS transmission

    //while ((IORD_ALTERA_AVALON_UART_STATUS(UART1_BASE) // uncomment for UART
    //        & ALTERA_AVALON_UART_STATUS_TRDY_MSK) != 0)
    while((IORD_ALTERA_AVALON_JTAG_UART_CONTROL(JTAG_UART_BASE) // JTAG UART
         & ALTERA_AVALON_JTAG_UART_CONTROL_WSPACE_MSK ) != 0 )
    {
        QF_INT_DISABLE();
        uint16_t b = QS::getByte();
        QF_INT_ENABLE();
        if (b != QS_EOD) {                   /* is the QS buffer NOT empty? */
            //IOWR_ALTERA_AVALON_UART_TXDATA(UART1_BASE, (uint8_t)b);
            IOWR_ALTERA_AVALON_JTAG_UART_DATA(JTAG_UART_BASE, (uint8_t)b);
        }
        else {
            break;         /* the QS buffer is empty, break out of the loop */
        }
    }

#endif                                                                // Q_SPY
}
//............................................................................
void QF::onCleanup(void) {
}
//............................................................................
void BSP_displyPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == (uint8_t)'e') {              // is this Philosopher eating?
        LED_ON(n);
    }
    else {                                   // this Philosopher is not eating
        LED_OFF(n);
    }
    QS_BEGIN(PHILO_STAT, QS::apObj_);
        QS_U8(1, n);
        QS_STR(stat);
    QS_END();
}
//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    LED_ON(7);
    QF_INT_DISABLE();                                // disable all interrupts
    for (;;) {      // NOTE: replace this endless loop in the production code!
    }
}

#ifdef Q_SPY
//----------------------------------------------------------------------------
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[QS_BUF_SIZE];               // buffer for Quantum Spy

    initBuf(qsBuf, sizeof(qsBuf));

    // Use the maximum period for the 32-bit timer
    IOWR_ALTERA_AVALON_TIMER_PERIODL(HIGH_RES_TIMER_BASE, 0xFFFF);
    IOWR_ALTERA_AVALON_TIMER_PERIODH(HIGH_RES_TIMER_BASE, 0xFFFF);
    // Start the timer running continuously
    IOWR_ALTERA_AVALON_TIMER_CONTROL(HIGH_RES_TIMER_BASE,
                                     ALTERA_AVALON_TIMER_CONTROL_CONT_MSK
                                     | ALTERA_AVALON_TIMER_CONTROL_START_MSK);

    // Disable receive and transmit interrupts for UART1
    // NOTE: baud rate is set by the hardware configuration
    //IOWR_ALTERA_AVALON_UART_CONTROL(UART1_BASE, 0);    // uncomment for UART

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

    return true;                      // indicate successful QS initialization
}
//............................................................................
void QS::onCleanup(void) {
}
//............................................................................
void QS::onFlush(void) {
    uint16_t b;
    while ((b = getByte()) != QS_EOD) {       // next QS trace byte available?
        //while ((IORD_ALTERA_AVALON_UART_STATUS(UART1_BASE)       // for UART
        //       & ALTERA_AVALON_UART_STATUS_TRDY_MSK) == 0)
        while ((IORD_ALTERA_AVALON_JTAG_UART_CONTROL(JTAG_UART_BASE) // JTAG
             & ALTERA_AVALON_JTAG_UART_CONTROL_WSPACE_MSK ) == 0 )
        {}
                                    /* write the byte to the TXDATA regiser */
        //IOWR_ALTERA_AVALON_UART_TXDATA(UART1_BASE, (uint8_t)b);  // for UART
        IOWR_ALTERA_AVALON_JTAG_UART_DATA( JTAG_UART_BASE, (uint8_t)b); //JTAG
    }
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {
    // latch the current timer value to the SNAPL and SNAPH registers
    IOWR_ALTERA_AVALON_TIMER_SNAPL(HIGH_RES_TIMER_BASE, 0);
    // read the snapshot from the SNAPL and SNAPH. Invert the timer value
    // to make it a count up timer.
    return ~((IORD_ALTERA_AVALON_TIMER_SNAPH(HIGH_RES_TIMER_BASE) << 16)
              | IORD_ALTERA_AVALON_TIMER_SNAPL(HIGH_RES_TIMER_BASE));
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------

