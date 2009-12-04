/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Nov 24, 2009
License:    Dual license model; choose one of two:
            Eclipse Public License - Version 1.0
            http://www.eclipse.org/legal/epl-v10.html
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 PacketPass, Inc and the Firebreath development team
\**********************************************************/

#include "PluginEvent.h"
#include "PluginEventSink.h"
#include "PluginEventSource.h"
#include "PluginEvents/AttachedEvent.h"

using namespace FB;

PluginEventSource::PluginEventSource()
{
}

PluginEventSource::~PluginEventSource()
{
}

void PluginEventSource::AttachObserver(FB::PluginEventSink *sink)
{
    m_observers.push_back(sink);
    AttachedEvent newEvent(true);
    sink->HandleEvent(&newEvent);
}

void PluginEventSource::DetachObserver(FB::PluginEventSink *sink)
{
    for (ObserverMap::iterator it = m_observers.begin(); it != m_observers.end(); it++) {
        if (*it == sink) {
            m_observers.erase(it);
            AttachedEvent newEvent(false);
            sink->HandleEvent(&newEvent);
            return;
        }
    }
}

bool PluginEventSource::SendEvent(PluginEvent* evt)
{
    for (ObserverMap::iterator it = m_observers.begin(); it != m_observers.end(); it++) {
        if ((*it)->HandleEvent(evt)) {
            return true;    // Tell the caller that the event was handled
        }
    }
    return false;
}