/**********************************************************\ 
Original Author: Richard Bateman and Georg Fritzsche 

Created:    December 3, 2009
License:    Dual license model; choose one of two:
            Eclipse Public License - Version 1.0
            http://www.eclipse.org/legal/epl-v10.html
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 PacketPass Inc, Georg Fritzsche,
               Firebreath development team
\**********************************************************/
#ifndef H_TEMPLATEPLUGIN
#define H_TEMPLATEPLUGIN

#include "PluginCore.h"

class TemplatePlugin : public FB::PluginCore
{
public:
    static void StaticInitialize();
    static void StaticDeinitialize();

public:
    TemplatePlugin();
    virtual ~TemplatePlugin();

public:
    virtual FB::JSAPI* createJSAPI();
    virtual bool IsWindowless() { return false; }
    virtual bool HandleEvent(FB::PluginEvent *evt);
};

#endif