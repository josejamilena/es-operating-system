// Generated by esidl (r1745).
// This file is expected to be modified for the Web IDL interface
// implementation.  Permission to use, copy, modify and distribute
// this file in any software license is hereby granted.

#ifndef ORG_W3C_DOM_BOOTSTRAP_MESSAGEEVENTIMP_H_INCLUDED
#define ORG_W3C_DOM_BOOTSTRAP_MESSAGEEVENTIMP_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <org/w3c/dom/html/MessageEvent.h>
#include "EventImp.h"

#include <org/w3c/dom/events/Event.h>
#include <org/w3c/dom/html/Window.h>
#include <org/w3c/dom/html/MessagePort.h>

namespace org
{
namespace w3c
{
namespace dom
{
namespace bootstrap
{
class MessageEventImp : public ObjectMixin<MessageEventImp, EventImp>
{
public:
    // MessageEvent
    Any getData() __attribute__((weak));
    std::u16string getOrigin() __attribute__((weak));
    std::u16string getLastEventId() __attribute__((weak));
    html::Window getSource() __attribute__((weak));
    html::MessagePortArray getPorts() __attribute__((weak));
    void initMessageEvent(std::u16string typeArg, bool canBubbleArg, bool cancelableArg, Any dataArg, std::u16string originArg, std::u16string lastEventIdArg, html::Window sourceArg, html::MessagePortArray portsArg) __attribute__((weak));
    // Object
    virtual Any message_(uint32_t selector, const char* id, int argc, Any* argv)
    {
        return html::MessageEvent::dispatch(this, selector, id, argc, argv);
    }
    static const char* const getMetaData()
    {
        return html::MessageEvent::getMetaData();
    }
};

}
}
}
}

#endif  // ORG_W3C_DOM_BOOTSTRAP_MESSAGEEVENTIMP_H_INCLUDED
