/*
 * Copyright (c) 2006, 2007
 * Nintendo Co., Ltd.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Nintendo makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

#include <es.h>
#include <es/handle.h>
#include <es/naming/IContext.h>
#include "arp.h"
#include "conduit.h"
#include "datagram.h"
#include "dix.h"
#include "icmp4.h"
#include "igmp.h"
#include "inet.h"
#include "inet4.h"
#include "inet4address.h"
#include "inet6.h"
#include "inet6address.h"
#include "inetConfig.h"
#include "loopback.h"
#include "resolver.h"
#include "tcp.h"
#include "udp.h"
#include "visualizer.h"

extern int esInit(IInterface** nameSpace);

Conduit* inProtocol;

void visualize()
{
    Visualizer v;
    inProtocol->accept(&v);
}

int main()
{
    IInterface* root = NULL;
    esInit(&root);
    Handle<IContext> context(root);

    Socket::initialize();

    esDump(&InAddrAny, 4);
    esDump(&InAddrLoopback, 4);
    esDump(&InAddrBroadcast, 4);

    u32 test = INADDR_LOOPBACK;
    esDump(&test, 4);

    esReport("DIXHdr: %d byte\n", sizeof(DIXHdr));
    ASSERT(sizeof(DIXHdr) == 14);

    // Check inchecksum concept
    s32 s = (u16) 0x1 + (u16) 0xfffe;
    esReport("%x\n", s);
    s = (s & 0xffff) + (s >> 16);
    esReport("%x\n", s);
    s = (u16) ~s;
    esReport("%x\n", s);    // s is always zero.

    // Setup internet protocol family
    InFamily* inFamily = new InFamily;
    esReport("AF: %d\n", inFamily->getAddressFamily());

    Socket raw(AF_INET, ISocket::Raw);
    inProtocol = inFamily->getProtocol(&raw);
    visualize();

    // Setup loopback interface
    Handle<IStream> loopbackStream = context->lookup("device/loopback");
    int scopeID = Socket::addInterface(loopbackStream, ARPHdr::HRD_LOOPBACK);

    // Register localhost address
    Handle<Inet4Address> localhost = new Inet4Address(InAddrLoopback, Inet4Address::statePreferred, scopeID, 8);
    inFamily->addAddress(localhost);
    localhost->start();
    visualize();

    // Create resolver object
    Resolver resolver;

    // Create internet config object
    InternetConfig ipconfig;

    // Test bind and connect operations
    Socket socket(AF_INET, ISocket::Datagram);
    socket.bind(localhost, 53);
    visualize();
    socket.connect(localhost, 53);
    visualize();

    // Test read and write operations
    char output[4] = "abc";
    socket.write(output, 4);
    char input[4];
    socket.read(input, 4);
    esReport("'%s'\n", input);

    // Test close operation
    socket.close();
    visualize();

    // Test local ping
    localhost->isReachable(10000000);

    // Setup DIX interface
    Handle<IStream> ethernetStream = context->lookup("device/ethernet");
    Handle<IEthernet> nic(ethernetStream);
    nic->start();
    int dixID = Socket::addInterface(ethernetStream, ARPHdr::HRD_ETHERNET);
    esReport("dixID: %d\n", dixID);

    // Register host address (192.168.2.40)
    InAddr addr = { htonl(192 << 24 | 168 << 16 | 2 << 8 | 40) };
    Handle<Inet4Address> host = resolver.getHostByAddress(&addr.addr, sizeof addr, dixID);
    ipconfig.addAddress(host, 16);
    visualize();

    // Wait for the host address to be settled.
    esSleep(80000000);
    ASSERT(host->isPreferred());

    ARPFamily* arpFamily = dynamic_cast<ARPFamily*>(Socket::getAddressFamily(AF_ARP));
    ASSERT(arpFamily);

#if 0
    // Test ARP to 192.168.2.20 (169.254.88.46)
    InAddr addrRemote = { htonl(192 << 24 | 168 << 16 | 2 << 8 | 20) };
    Handle<Inet4Address> dst = new Inet4Address(addrRemote, Inet4Address::stateInit, dixID);
    inFamily->addAddress(dst);
    arpFamily->addAddress(dst);
    dst->start();
#endif

    // Register a default router (192.168.2.1)
    InAddr addrRouter = { htonl(192 << 24 | 168 << 16 | 2 << 8 | 1) };
    Handle<Inet4Address> router = resolver.getHostByAddress(&addrRouter.addr, sizeof addr, dixID);
    ipconfig.addRouter(router);

#if 0
    esReport("ping #1\n");
    router->isReachable(10000000);
    esReport("ping #2\n");
    router->isReachable(10000000);
    esReport("ping #3\n");
    router->isReachable(10000000);
#endif

    // Test remote ping (192.195.204.26 / www.nintendo.com)
    InAddr addrRemote = { htonl(192 << 24 | 195 << 16 | 204 << 8 | 26) };
    Handle<Inet4Address> remote = resolver.getHostByAddress(&addrRemote.addr, sizeof addr, 0);
    esReport("ping #1\n");
    remote->isReachable(10000000);
    esReport("ping #2\n");
    remote->isReachable(10000000);
    esReport("ping #3\n");
    remote->isReachable(10000000);

    esSleep(100000000);
    nic->stop();

    esReport("done.\n");
}
