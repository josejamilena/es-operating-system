/*
 * Copyright 2008, 2009 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sample.h"
#include "proxyImpl.h"

#include "w3c/dom.h"
#include "w3c/events.h"
#include "w3c/stylesheets.h"
#include "w3c/cssomview.h"
#include "w3c/css.h"
#include "w3c/ranges.h"
#include "w3c/traversal.h"
#include "w3c/ls.h"
#include "w3c/html5.h"
#include "w3c/smil.h"
#include "w3c/svg.h"

#include "w3c/dom.template.h"
#include "w3c/events.template.h"
#include "w3c/stylesheets.template.h"
#include "w3c/cssomview.template.h"
#include "w3c/css.template.h"
#include "w3c/ranges.template.h"
#include "w3c/traversal.template.h"
#include "w3c/ls.template.h"
#include "w3c/html5.template.h"
#include "w3c/smil.template.h"
#include "w3c/svg.template.h"

#include <new>

char* NPP_GetMIMEDescription()
{
    return const_cast<char*>("application/es-npapi-sample:none:ES NPAPI sample plugin");
}

// Invoked from NP_Initialize()
NPError NPP_Initialize()
{
    printf("%s\n", __func__);

    addInterfaceData(es::Node::iid(),
                     es::Node::info());
    addProxyConstructor(es::Node::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::Node_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::CharacterData::iid(),
                     es::CharacterData::info());
    addProxyConstructor(es::CharacterData::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::CharacterData_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::Text::iid(),
                     es::Text::info());
    addProxyConstructor(es::Text::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::Text_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::Document::iid(),
                     es::Document::info());
    addProxyConstructor(es::Document::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::Document_Proxy<Object*, invoke> >::createInstance));

    // This steps should be automated with es::Document.
    addInterfaceData("::es::HTMLDocument",
                     es::Document::info());
    addProxyConstructor("::es::HTMLDocument",
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::Document_Proxy<Object*, invoke> >::createInstance));

    // This steps should be automated with es::Document.
    addInterfaceData("::es::XMLDocument",
                     es::Document::info());
    addProxyConstructor("::es::XMLDocument",
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::Document_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::Element::iid(),
                     es::Element::info());
    addProxyConstructor(es::Element::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::Element_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::HTMLElement::iid(),
                     es::HTMLElement::info());
    addProxyConstructor(es::HTMLElement::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::HTMLElement_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::HTMLBodyElement::iid(),
                     es::HTMLBodyElement::info());
    addProxyConstructor(es::HTMLBodyElement::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::HTMLBodyElement_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::HTMLDivElement::iid(),
                     es::HTMLDivElement::info());
    addProxyConstructor(es::HTMLDivElement::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::HTMLDivElement_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::HTMLCanvasElement::iid(),
                     es::HTMLCanvasElement::info());
    addProxyConstructor(es::HTMLCanvasElement::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::HTMLCanvasElement_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::CanvasRenderingContext2D::iid(),
                     es::CanvasRenderingContext2D::info());
    addProxyConstructor(es::CanvasRenderingContext2D::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::CanvasRenderingContext2D_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::SVGElement::iid(),
                     es::SVGElement::info());
    addProxyConstructor(es::SVGElement::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::SVGElement_Proxy<Object*, invoke> >::createInstance));

    addInterfaceData(es::SVGCircleElement::iid(),
                     es::SVGCircleElement::info());
    addProxyConstructor(es::SVGCircleElement::iid(),
                        reinterpret_cast<Object* (*)(ProxyObject)>(Proxy_Impl<ProxyObject, es::SVGCircleElement_Proxy<Object*, invoke> >::createInstance));

     return NPERR_NO_ERROR;
}

// Invoked from NP_Shutdown()
void NPP_Shutdown()
{
    printf("%s\n", __func__);
}

NPError NPP_New(NPMIMEType pluginType, NPP npp, uint16_t mode,
                int16_t argc, char* argn[], char* argv[],
                NPSavedData* saved)
{
    printf("%s\n", __func__);
    if (!npp)
    {
        return NPERR_INVALID_INSTANCE_ERROR;
    }
    npp->pdata = new (std::nothrow) PluginInstance(npp);
    if (!npp->pdata)
    {
        return NPERR_INVALID_INSTANCE_ERROR;
    }
    return NPERR_NO_ERROR;
}

NPError NPP_Destroy(NPP npp, NPSavedData** save)
{
    printf("%s\n", __func__);
    if (npp == NULL)
    {
        return NPERR_INVALID_INSTANCE_ERROR;
    }
    PluginInstance* instance = static_cast<PluginInstance*>(npp->pdata);
    if (instance)
    {
        delete instance;
        npp->pdata = 0;
    }
    return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP npp, NPWindow* window)
{
    printf("%s\n", __func__);
    if (!npp)
    {
        return NPERR_INVALID_INSTANCE_ERROR;
    }
    if (!window)
    {
        return NPERR_GENERIC_ERROR;
    }
    if (!npp->pdata)
    {
        return NPERR_GENERIC_ERROR;
    }
    return NPERR_NO_ERROR;
}

NPObject* NPP_GetScriptableInstance(NPP npp)
{
    printf("%s\n", __func__);
    if (!npp)
    {
        return 0;
    }
    PluginInstance* instance = static_cast<PluginInstance*>(npp->pdata);
    if (instance)
    {
        return instance->getScriptableInstance();
    }
    return 0;
}

NPError NPP_GetValue(NPP npp, NPPVariable variable, void* value)
{
    printf("%s\n", __func__);
    if (!npp)
    {
        return NPERR_INVALID_INSTANCE_ERROR;
    }
    switch (variable)
    {
    case NPPVpluginNameString:
    case NPPVpluginDescriptionString:
        *reinterpret_cast<const char**>(value) = "ES NPAPI sample plugin";
        break;
    case NPPVpluginScriptableNPObject:
        *reinterpret_cast<NPObject**>(value) = NPP_GetScriptableInstance(npp);
        if (*reinterpret_cast<NPObject**>(value))
        {
            return NPERR_GENERIC_ERROR;
        }
        break;
    default:
        break;
    }
    return NPERR_NO_ERROR;
}

int16_t NPP_HandleEvent(NPP npp, void* event)
{
    return 0;
}

void NPP_StreamAsFile(NPP npp, NPStream* stream, const char* fname)
{
    printf("%s\n", __func__);
}
