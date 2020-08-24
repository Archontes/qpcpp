//.$file${.::defer.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: defer.qm
// File:  ${.::defer.cpp}
//
// This code has been generated by QM 5.0.3 <www.state-machine.com/qm/>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
//.$endhead${.::defer.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "qpcpp.hpp"
#include "bsp.hpp"

#include "safe_std.h" // portable "safe" <stdio.h>/<string.h> facilities

Q_DEFINE_THIS_FILE

//............................................................................
enum TServerSignals {
    NEW_REQUEST_SIG = QP::Q_USER_SIG, // the new request signal
    RECEIVED_SIG,     // the request has been received
    AUTHORIZED_SIG,   // the request has been authorized
    TERMINATE_SIG     // terminate the application
};
//............................................................................
//.$declare${Events::RequestEvt} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${Events::RequestEvt} .....................................................
class RequestEvt : public QP::QEvt {
public:
    uint8_t ref_num;
};
//.$enddecl${Events::RequestEvt} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// Active object class -----------------------------------------------------..
//.$declare${Components::TServer} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${Components::TServer} ....................................................
class TServer : public QP::QActive {
private:

    // native QF queue for deferred request events
    QP::QEQueue m_requestQueue;

    // storage for the deferred queue buffer
    QP::QEvt const * m_requestQSto[3];

    // request event being processed
    RequestEvt const * m_activeRequest;

    // private time event generator
    QP::QTimeEvt m_receivedEvt;

    // private time event generator
    QP::QTimeEvt m_authorizedEvt;

public:
    static TServer inst;

public:
    explicit TServer() noexcept
      : QActive(Q_STATE_CAST(&initial)),
        m_receivedEvt(this, RECEIVED_SIG),
        m_authorizedEvt(this, AUTHORIZED_SIG)
    {
        m_requestQueue.init(m_requestQSto, Q_DIM(m_requestQSto));
    }

protected:
    Q_STATE_DECL(initial);
    Q_STATE_DECL(idle);
    Q_STATE_DECL(busy);
    Q_STATE_DECL(receiving);
    Q_STATE_DECL(authorizing);
    Q_STATE_DECL(final);
};
//.$enddecl${Components::TServer} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//.$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//. Check for the minimum required QP version
#if (QP_VERSION < 680U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.8.0 or higher required
#endif
//.$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//.$define${Components::TServer} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//.${Components::TServer} ....................................................
TServer TServer::inst;
//.${Components::TServer::SM} ................................................
Q_STATE_DEF(TServer, initial) {
    //.${Components::TServer::SM::initial}
    (void)e; // unused parameter
    m_activeRequest = nullptr; // no active request yet

    QS_OBJ_DICTIONARY(&TServer::inst);
    QS_OBJ_DICTIONARY(&TServer::inst.m_receivedEvt);
    QS_OBJ_DICTIONARY(&TServer::inst.m_authorizedEvt);
    QS_OBJ_DICTIONARY(&TServer::inst.m_requestQueue);

    QS_FUN_DICTIONARY(&TServer::idle);
    QS_FUN_DICTIONARY(&TServer::busy);
    QS_FUN_DICTIONARY(&TServer::receiving);
    QS_FUN_DICTIONARY(&TServer::authorizing);
    QS_FUN_DICTIONARY(&TServer::final);

    return tran(&idle);
}
//.${Components::TServer::SM::idle} ..........................................
Q_STATE_DEF(TServer, idle) {
    QP::QState status_;
    switch (e->sig) {
        //.${Components::TServer::SM::idle}
        case Q_ENTRY_SIG: {
            PRINTF_S("%s\n", "idle-ENTRY;");

            // recall the oldest deferred request...
            if (recall(&m_requestQueue)) {
                PRINTF_S("%s\n", "Request recalled");
            }
            else {
                PRINTF_S("%s\n", "No deferred requests");
            }
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${Components::TServer::SM::idle::NEW_REQUEST}
        case NEW_REQUEST_SIG: {
            // create and save a new reference to the request event so that
            // this event will be available beyond this RTC step and won't be
            // recycled.
            Q_NEW_REF(m_activeRequest, RequestEvt);

            PRINTF_S("Processing request #%d\n",
                   (int)m_activeRequest->ref_num);
            status_ = tran(&receiving);
            break;
        }
        //.${Components::TServer::SM::idle::TERMINATE}
        case TERMINATE_SIG: {
            status_ = tran(&final);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.${Components::TServer::SM::busy} ..........................................
Q_STATE_DEF(TServer, busy) {
    QP::QState status_;
    switch (e->sig) {
        //.${Components::TServer::SM::busy}
        case Q_EXIT_SIG: {
            PRINTF_S("busy-EXIT; done processing request #%d\n",
                   (int)m_activeRequest->ref_num);

            // delete the reference to the active request, because
            // it is now processed.
            Q_DELETE_REF(m_activeRequest);
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${Components::TServer::SM::busy::NEW_REQUEST}
        case NEW_REQUEST_SIG: {
            // defer the new request event...
            if (defer(&m_requestQueue, e)) {
                PRINTF_S("Request #%d deferred;\n",
                       (int)Q_EVT_CAST(RequestEvt)->ref_num);
            }
            else {
                // notify the request sender that his request was denied...
                PRINTF_S("Request #%d IGNORED;\n",
                       (int)Q_EVT_CAST(RequestEvt)->ref_num);
            }
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${Components::TServer::SM::busy::TERMINATE}
        case TERMINATE_SIG: {
            status_ = tran(&final);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.${Components::TServer::SM::busy::receiving} ...............................
Q_STATE_DEF(TServer, receiving) {
    QP::QState status_;
    switch (e->sig) {
        //.${Components::TServer::SM::busy::receiving}
        case Q_ENTRY_SIG: {
            // inform about the first stage of processing of the request...
            PRINTF_S("receiving-ENTRY; active request: #%d\n",
                   (int)m_activeRequest->ref_num);

            // one-shot timeout in 1 second
            m_receivedEvt.armX(BSP_TICKS_PER_SEC, 0U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${Components::TServer::SM::busy::receiving}
        case Q_EXIT_SIG: {
            m_receivedEvt.disarm();
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${Components::TServer::SM::busy::receiving::RECEIVED}
        case RECEIVED_SIG: {
            status_ = tran(&authorizing);
            break;
        }
        default: {
            status_ = super(&busy);
            break;
        }
    }
    return status_;
}
//.${Components::TServer::SM::busy::authorizing} .............................
Q_STATE_DEF(TServer, authorizing) {
    QP::QState status_;
    switch (e->sig) {
        //.${Components::TServer::SM::busy::authorizing}
        case Q_ENTRY_SIG: {
            // inform about the second stage of processing of the request..
            PRINTF_S("authorizing-ENTRY; active request: #%d\n",
                   (int)m_activeRequest->ref_num);

            // one-shot timeout in 2 seconds
            m_authorizedEvt.armX(2U*BSP_TICKS_PER_SEC, 0U);
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${Components::TServer::SM::busy::authorizing}
        case Q_EXIT_SIG: {
            m_authorizedEvt.disarm();
            status_ = Q_RET_HANDLED;
            break;
        }
        //.${Components::TServer::SM::busy::authorizing::AUTHORIZED}
        case AUTHORIZED_SIG: {
            status_ = tran(&idle);
            break;
        }
        default: {
            status_ = super(&busy);
            break;
        }
    }
    return status_;
}
//.${Components::TServer::SM::final} .........................................
Q_STATE_DEF(TServer, final) {
    QP::QState status_;
    switch (e->sig) {
        //.${Components::TServer::SM::final}
        case Q_ENTRY_SIG: {
            PRINTF_S("%s\n", "final-ENTRY;");
                     QP::QF::stop(); // terminate the application
            status_ = Q_RET_HANDLED;
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}
//.$enddef${Components::TServer} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// test harness ==============================================================

// Local-scope objects -------------------------------------------------------
static QP::QEvt const *l_tserverQSto[10]; // Event queue storage for TServer
static QF_MPOOL_EL(RequestEvt) l_smlPoolSto[20]; // storage for small pool

//............................................................................
int main(int argc, char *argv[]) {
    PRINTF_S("Deferred Event state pattern\nQP version: %s\n"
           "Press 'n' to generate a new request\n"
           "Press ESC to quit...\n",
           QP_VERSION_STR);

    QP::QF::init(); // initialize the framework and the underlying RTOS/OS

    BSP_init(argc, argv); // initialize the BSP


    // publish-subscribe not used, no call to QF_psInit()

    // initialize event pools...
    QP::QF::poolInit(l_smlPoolSto,
                     sizeof(l_smlPoolSto), sizeof(l_smlPoolSto[0]));

    QS_SIG_DICTIONARY(NEW_REQUEST_SIG, nullptr); // global signals
    QS_SIG_DICTIONARY(RECEIVED_SIG,    nullptr);
    QS_SIG_DICTIONARY(AUTHORIZED_SIG,  nullptr);
    QS_SIG_DICTIONARY(TERMINATE_SIG,   nullptr);

    // start the active objects...
    TServer::inst.start(1U,
                    l_tserverQSto, Q_DIM(l_tserverQSto),
                    nullptr, 0U);

    return QP::QF::run(); // run the QF application
}
//............................................................................
void BSP_onKeyboardInput(uint8_t key) {
    switch (key) {
        case 'n': {  // 'n': new request?
            static uint8_t reqCtr = 0; // count the requests
            RequestEvt *e = Q_NEW(RequestEvt, NEW_REQUEST_SIG);
            e->ref_num = (++reqCtr); // set the reference number
            // post directly to TServer active object
            TServer::inst.POST(e, (void *)0);
            break;
        }
        case '\33': { // ESC pressed?
            static QP::QEvt const terminateEvt { TERMINATE_SIG, 0U, 0U };
            TServer::inst.POST(&terminateEvt, (void *)0);
            break;
        }
    }
}
