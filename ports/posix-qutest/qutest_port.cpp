/// @file
/// @brief QUTEST port for POSIX
/// @ingroup ports
/// @cond
///***************************************************************************
/// Last updated for version 6.3.7
/// Last updated on  2018-11-09
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2018 Quantum Leaps, LLC. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <http://www.gnu.org/licenses/>.
///
/// Contact information:
/// https://www.state-machine.com
/// mailto:info@state-machine.com
///***************************************************************************
/// @endcond
///
#ifndef Q_SPY
    #error "Q_SPY must be defined for QUTEST application"
#endif // Q_SPY

#define QP_IMPL       // this is QP implementation
#include "qf_port.h"  // QF port
#include "qassert.h"  // QP embedded systems-friendly assertions
#include "qs_port.h"  // include QS port

#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>

#define QS_TX_SIZE     (8*1024)
#define QS_RX_SIZE     (2*1024)
#define QS_TX_CHUNK    QS_TX_SIZE
#define QS_IMEOUT_MS   100

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

namespace QP {

//Q_DEFINE_THIS_MODULE("qutest_port")

// local variables ...........................................................
static int l_sock = INVALID_SOCKET;
static void sigIntHandler(int dummy);

//............................................................................
bool QS::onStartup(void const *arg) {
    static uint8_t qsBuf[QS_TX_SIZE];   // buffer for QS-TX channel
    static uint8_t qsRxBuf[QS_RX_SIZE]; // buffer for QS-RX channel
    char hostName[128];
    char const *serviceName = "6601";  /* default QSPY server port */
    char const *src;
    char *dst;
    int status;

    struct addrinfo *result = NULL;
    struct addrinfo *rp = NULL;
    struct addrinfo hints;
    int sockopt_bool;
    struct sigaction sig_act;

    // initialize the QS transmit and receive buffers
    initBuf(qsBuf, sizeof(qsBuf));
    rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    // extract hostName from 'arg' (hostName:port_remote)...
    src = (arg != static_cast<void const *>(0))
          ? static_cast<char const *>(arg)
          : "localhost"; // default QSPY host
    dst = hostName;
    while ((*src != '\0')
           && (*src != ':')
           && (dst < &hostName[sizeof(hostName) - 1]))
    {
        *dst++ = *src++;
    }
    *dst = '\0'; // zero-terminate hostName

    // extract port_remote from 'arg' (hostName:port_remote)...
    if (*src == ':') {
        serviceName = src + 1;
    }
    //printf("<TARGET> Connecting to QSPY on Host=%s:%s...\n",
    //       hostName, serviceName);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    status = getaddrinfo(hostName, serviceName, &hints, &result);
    if (status != 0) {
        fprintf(stderr,
            "<TARGET> ERROR   cannot resolve host Name=%s:%s,Err=%d\n",
                    hostName, serviceName, status);
        goto error;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        l_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (l_sock != INVALID_SOCKET) {
            if (connect(l_sock, rp->ai_addr, rp->ai_addrlen)
                == SOCKET_ERROR)
            {
                close(l_sock);
                l_sock = INVALID_SOCKET;
            }
            break;
        }
    }

    freeaddrinfo(result);

    // socket could not be opened & connected?
    if (l_sock == INVALID_SOCKET) {
        fprintf(stderr, "<TARGET> ERROR   cannot connect to QSPY at "
            "host=%s:%s\n",
            hostName, serviceName);
        goto error;
    }

    // set the socket to non-blocking mode
    status = fcntl(l_sock, F_GETFL, 0);
    if (status == -1) {
        fprintf(stderr,
            "<TARGET> ERROR   Socket configuration failed errno=%d\n",
            errno);
        QS_EXIT();
        goto error;
    }
    if (fcntl(l_sock, F_SETFL, status | O_NONBLOCK) != 0) {
        fprintf(stderr, "<TARGET> ERROR   Failed to set non-blocking socket "
            "errno=%d\n", errno);
        QS_EXIT();
        goto error;
    }

    /* configure the socket to reuse the address and not to linger */
    sockopt_bool = 1;
    setsockopt(l_sock, SOL_SOCKET, SO_REUSEADDR,
               &sockopt_bool, sizeof(sockopt_bool));
    sockopt_bool = 0; /* negative option */
    setsockopt(l_sock, SOL_SOCKET, SO_LINGER,
               &sockopt_bool, sizeof(sockopt_bool));

    //printf("<TARGET> Connected to QSPY at Host=%s:%d\n",
    //       hostName, port_remote);
    onFlush();

    // install the SIGINT (Ctrl-C) signal handler
    sig_act.sa_handler = &sigIntHandler;
    sigaction(SIGINT, &sig_act, NULL);

    return true;  // success

error:
    return false; // failure
}
//............................................................................
void QS::onCleanup(void) {
    if (l_sock != INVALID_SOCKET) {
        close(l_sock);
        l_sock = INVALID_SOCKET;
    }
    //printf("<TARGET> Disconnected from QSPY\n");
}
//............................................................................
void QS::onReset(void) {
    onCleanup();
    exit(0);
}
//............................................................................
void QS::onFlush(void) {
    uint16_t nBytes;
    uint8_t const *data;

    if (l_sock == INVALID_SOCKET) { // socket initialized?
        return;
    }

    nBytes = QS_TX_CHUNK;
    while ((data = getBlock(&nBytes)) != (uint8_t *)0) {
        int nSent = send(l_sock, (char const *)data, (int)nBytes, 0);
        // the driver buffers the output, so it should accept all the bytes
        if (nSent < (int)nBytes) {
            fprintf(stderr, "<TARGET> ERROR   sending data over TCP,"
                   "errno=%d\n", errno);
        }
        nBytes = QS_TX_CHUNK;
    }
}
//............................................................................
void QS::onTestLoop() {
    fd_set readSet;
    FD_ZERO(&readSet);

    rxPriv_.inTestLoop = true;
    while (rxPriv_.inTestLoop) {
        static struct timeval timeout = {
            (long)0, (long)(QS_IMEOUT_MS * 1000)
        };
        int nrec;
        uint16_t nBytes;
        uint8_t const *block;

        FD_SET(l_sock, &readSet);   // the socket

        // selective, timed blocking on the TCP/IP socket...
        timeout.tv_usec = (long)(QS_IMEOUT_MS * 1000);
        nrec = select(l_sock + 1, &readSet,
                      (fd_set *)0, (fd_set *)0, &timeout);
        if (nrec < 0) {
            fprintf(stderr,
                "<TARGET> ERROR socket select,errno=%d\n",
                errno);
            onCleanup();
            exit(-2);
        }
        else if (nrec > 0) {
            if (FD_ISSET(l_sock, &readSet)) { // socket ready to read?
                uint8_t buf[QS_RX_SIZE];
                int status = recv(l_sock, (char *)buf, (int)sizeof(buf), 0);
                while (status > 0) { // any data received?
                    uint8_t *pb;
                    int i = (int)rxGetNfree();
                    if (i > status) {
                        i = status;
                    }
                    status -= i;
                    // reorder the received bytes into QS-RX buffer
                    for (pb = &buf[0]; i > 0; --i, ++pb) {
                        rxPut(*pb);
                    }
                    rxParse(); // parse all n-bytes of data
                }
            }
        }

        nBytes = QS_TX_SIZE;
        //QF_CRIT_ENTRY(dummy);
        block = getBlock(&nBytes);
        //QF_CRIT_EXIT(dummy);

        if (block != (uint8_t *)0) {
            send(l_sock, (char const *)block, nBytes, 0);
        }
    }
    // set inTestLoop to true in case calls to QS_onTestLoop() nest,
    // which can happen through the calls to QS_TEST_PAUSE().
    rxPriv_.inTestLoop = true;
}
//............................................................................
static void sigIntHandler(int /*dummy*/) {
    QS::onCleanup();
    //printf("\n<TARGET> disconnecting from QSPY\n");
    exit(-1);
}

} // namespace QP

