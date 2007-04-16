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

#include "apic.h"
#include "core.h"
#include "io.h"
#include "rtc.h"

extern long hzBus;

u8 Apic::idBSP = 0;             // boot-strap processor's local APIC id
volatile u32* Apic::localApic;  // memory-mapped address of local APIC
volatile bool Apic::online;
unsigned Apic::busClock;

void Apic::
setIoApicID(volatile u32* addr, u8 id)
{
    addr[IOREGSEL] = IOAPICID;
    u32 ioapicid = addr[IOWIN];
    ioapicid &= 0xf0ffffff;
    ioapicid |= (id & 0x0f) << 24;
    addr[IOWIN] = ioapicid;
}

void Apic::
enableLocalApic()
{
    u8 id = getLocalApicID();
    localApic[SVR] |= 0x100;
    localApic[DFR] = 0xffffffff;   // Using the flat model
    localApic[LDR] &= ~(0xff << 24);
    localApic[LDR] |= 1 << (id + 24);

    localApic[LVT_LINT0] = localApic[LVT_LINT0] | 0x10000;  // masked
    localApic[LVT_LINT1] = localApic[LVT_LINT1] | 0x10000;  // masked
    localApic[LVT_TMR] = localApic[LVT_TMR] | 0x10000;      // masked
    localApic[LVT_PCR] = localApic[LVT_PCR] | 0x10000;      // masked
}

u8 Apic::
getLocalApicVersion()
{
    return localApic[APICVER] & 0xff;
}

void Apic::
setImcr(u8 value)
{
    Mps::FloatingPointerStructure* fps = mps->getFloatingPointerStructure();
    if (fps->featureBytes[1] & 0x80)    // test IMCRP bit
    {
        outpb(0x22, 0x70);
        outpb(0x23, value);
    }
}

Apic::
Apic(Mps* mps) :
    mps(mps)
{
    if (!mps)
    {
        return;
    }

    Mps::ConfigurationTableHeader* cth = mps->getConfigurationTableHeader();
    if (!cth)
    {
        localApic = reinterpret_cast<u32*>(0xfee00000);
    }
    else
    {
        localApic = reinterpret_cast<u32*>(cth->localApicAddress);
    }

    setImcr(0x01);  // Switch to symmetric I/O mode

    enableLocalApic();

    if (cth)
    {
        idBSP = getLocalApicID();
        setIoApicID(reinterpret_cast<u32*>(0xfec00000), 2);
    }
    else
    {
        Setup setup(this);
        cth->accept(setup);
    }
    esReport("BSP: %u, Local APIC %x\n", idBSP, localApic);
}

Apic::
~Apic()
{
}

void Apic::
startup(unsigned irq)
{
    setAffinity(irq, 0xff);
    enable(irq);
}

void Apic::
shutdown(unsigned irq)
{
    disable(irq);
}

void Apic::
enable(unsigned int irq)
{
    enable(irq, 32 + irq);
}

void Apic::
enable(unsigned int irq, u8 vec)
{
    if (vec < 0x10 || 0xfe < vec)
    {
        return;
    }

    Mps::InterruptAssignment assignment;
    volatile u32* addr = mps->getInterruptAssignment(irq, assignment);
    if (!addr || 23 < assignment.apicINTINn)
    {
        return;
    }

    addr[IOREGSEL] = IOREDTBL + 2 * assignment.apicINTINn;
    addr[IOWIN] = (1 << 11) | (1 << 8) | vec;   // logical, lowest priority
}

void Apic::
disable(unsigned int irq)
{
    disable(irq, 32 + irq);
}

void Apic::
disable(unsigned int irq, u8 vec)
{
    if (vec < 0x10 || 0xfe < vec)
    {
        return;
    }

    Mps::InterruptAssignment assignment;
    volatile u32* addr = mps->getInterruptAssignment(irq, assignment);
    if (!addr || 23 < assignment.apicINTINn)
    {
        return;
    }
    addr[IOREGSEL] = IOREDTBL + 2 * assignment.apicINTINn;

    addr[IOWIN] = 1 << 16;  // mask
}

bool Apic::
ack(unsigned int irq)
{
    u8 vec = 32 + irq;
    if (vec < 0x10 || 0xfe < vec)
    {
        return false;
    }

    // XXX disable(irq, vec);

    // Note it appears qemu does not emulate ISR and we don't check ISR here.
    localApic[EOI] = 0;

    return true;
}

void Apic::
end(unsigned int irq)
{
    // XXX enable(irq);
}

void Apic::
setAffinity(unsigned int irq, unsigned int mask)
{
    Mps::InterruptAssignment assignment;
    volatile u32* addr = mps->getInterruptAssignment(irq, assignment);
    if (!addr || 23 < assignment.apicINTINn)
    {
        return;
    }

    addr[IOREGSEL] = IOREDTBL + 2 * assignment.apicINTINn + 1;
    addr[IOWIN] = (mask << 24);
}

// cf. HLT opcode is 0xf4
void Apic::
sendInit(u8 id, u32 addr)
{
    u32 r;
    long t;

    // Set shutdown code and warm-reset vector
    Rtc::cmosWrite(0x0f, 0x0a);
    *(u32*) ((0x40 << 4) | 0x67) = ((addr & 0xffff0) << 12) | (addr & 0x0f);

    // set the target AP id in ICRH
    r = localApic[ICRH];
    r &= 0xF0FFFFFF;
    r |= id << (56 - 32);
    localApic[ICRH] = r;

Retry:
    // Send a INIT message to the AP
    r = localApic[ICRL];
    r &= 0xFFF0F800;
    r |= 0x00000500;
    localApic[ICRL] = r;

    // Check delivery status for 20 microseconds
    esSleep(200);
    if (localApic[ICRL] & (1u << 12))
    {
        // Failed
        goto Retry;
    }
}

// addr must be page-aligned
void Apic::
sendStartup(u8 id, u32 addr)
{
    u32 r;
    long t;

    ASSERT((addr & ~0x000ff000) == 0);

    // Set the target AP id in ICRH
    r = localApic[ICRH];
    r &= 0xF0FFFFFF;
    r |= id << (56 - 32);
    localApic[ICRH] = r;

Retry:
    // Send a start-up IPI message to the AP
    r = localApic[ICRL];
    r &= 0xFFF0F800;
    r |= 0x00000600 | (addr >> 12);
    localApic[ICRL] = r;

    // Check delivery status for 20 microseconds
    esSleep(200);
    if (localApic[ICRL] & (1u << 12))
    {
        // Failed
        goto Retry;
    }
}

/** Broadcasts an IPI message to all excluding self
 */
void Apic::
broadcastIPI(u8 vec)
{
    if (!localApic)
    {
        return;
    }

    while (localApic[ICRL] & 0x1000)
    {
        // Wait during send pending
    }
    u32 r = localApic[ICRL];
    r &= 0xFFF0F800;
    r |= 0x000c4000 | vec;
    localApic[ICRL] = r;
}

void Apic::
startupAp(u8 id, u32 hltAP, u32 startAP)
{
    online = false;

    if (getLocalApicVersion() < 0x10)
    {
        // 82489DX external APIC
        sendInit(id, startAP);
    }
    else
    {
        // local APIC
        sendInit(id, hltAP);
        esSleep(100000);

        sendStartup(id, startAP);
        esSleep(2000);

        sendStartup(id, startAP);
        esSleep(2000);
    }

    // Wait till AP gets ready
    while (!online)
    {
        __asm__ __volatile__ ("pause\n");
    }
}

void Apic::
shutdownAp(u8 id, u32 hltAP)
{
#if 0
    setTimer(32, 0);   // stop timer
#endif
    sendInit(id, hltAP);
    esSleep(400000);
}

unsigned Apic::
splIdle()
{
    register unsigned eax;
    __asm__ __volatile__ (
        "pushfl\n"
        "popl    %0\n"
        "sti"
        : "=a" (eax));
    return eax;
}

unsigned Apic::
splLo()
{
    register unsigned eax;
    __asm__ __volatile__ (
        "pushfl\n"
        "popl   %0\n"
        "sti"
        : "=a" (eax));
    return eax;
}

unsigned Apic::
splHi()
{
    register unsigned eax;
    __asm__ __volatile__ (
        "pushfl\n"
        "popl   %0\n"
        "cli"
        : "=a" (eax));
    return eax;
}

void Apic::
splX(unsigned x)
{
    __asm__ __volatile__ (
        "pushl   %0\n"
        "popfl"
        :: "r" (x));
}

//
// IInterface
//

bool Apic::
queryInterface(const Guid& riid, void** objectPtr)
{
    if (riid == IID_IPic)
    {
        *objectPtr = static_cast<IPic*>(this);
    }
    else if (riid == IID_IInterface)
    {
        *objectPtr = static_cast<IPic*>(this);
    }
    else
    {
        *objectPtr = NULL;
        return false;
    }
    static_cast<IInterface*>(*objectPtr)->addRef();
    return true;
}

unsigned int Apic::
addRef(void)
{
    return ref.addRef();
}

unsigned int Apic::
release(void)
{
    unsigned int count = ref.release();
    if (count == 0)
    {
        delete this;
        return 0;
    }
    return count;
}

int Apic::
readRtcCounter(int addr)
{
    do
    {
        outpb(Rtc::PORT_ADDR, Rtc::PORT_A);
    }
    while (inpb(Rtc::PORT_DATA) & 0x80);    // wait UIP
    outpb(Rtc::PORT_ADDR, addr);
    u8 bcd = inpb(Rtc::PORT_DATA);
    return (bcd & 0xf) + 10 * (bcd >> 4);
}

// Determine bus clock frequency
void Apic::
busFreq()
{
    int t0;
    int t1;
    int t2;

    localApic[DCR] &= ~0x0b;
    localApic[DCR] |= 0x0b;         // divide by 1

    localApic[LVT_TR] |= 0x10000;   // mask
    localApic[LVT_TR] &= ~0x20000;  // one-shot

    unsigned x = Core::splHi();
    t0 = readRtcCounter(Rtc::SECONDS);
    do {
        t1 = readRtcCounter(Rtc::SECONDS);
    } while (t0 == t1);
    localApic[ICR] = 0xffffffff;
    do {
        t2 = readRtcCounter(Rtc::SECONDS);
    } while (t2 == t1);
    unsigned count = localApic[CCR];
    Core::splX(x);

    busClock = 0xffffffffU - count;
    busClock = (busClock + 500000) / 1000000 * 1000000;
    esReport("Bus freq: %u %u\n", busClock, 0xffffffffU - count);

    localApic[ICR] = 1;
    while (0 < localApic[CCR])
    {
    }
}

// To stop timer, set hz = 0
void Apic::
setTimer(int vec, long hz)
{
    ASSERT(vec < 256);
    ASSERT(0 < busClock);
    localApic[LVT_TR] |= 0x10000;   // mask
    localApic[LVT_TR] &= ~0x20000;  // one-shot
    if (0 < hz)
    {
        localApic[DCR] &= ~0x0b;
        localApic[DCR] |= 0x0b;                 // divide by 1
        localApic[ICR] = busClock / hz;
        localApic[LVT_TR] |= 0x20000 | vec;     // periodic
        localApic[LVT_TR] &= ~0x10000;          // unmask
    }
}
