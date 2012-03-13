/**
 * @file Dispatch.cpp
 * @author The VLE Development Team
 * See the AUTHORS or Authors.txt file
 */

/*
 * Copyright (C) 2012 ULCO http://www.univ-littoral.fr
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vle/devs/Dynamics.hpp>
#include <Container.hpp>
#include <list>

namespace logistics {

class Dispatch : public vle::devs::Dynamics
{
public:
    Dispatch(const vle::devs::DynamicsInit& init,
             const vle::devs::InitEventList& events) :
        vle::devs::Dynamics(init, events)
    {
    }

    vle::devs::ExternalEvent* cloneExternalEvent(
        vle::devs::ExternalEvent* event, const std::string& portName) const
    {
        vle::devs::ExternalEvent* ee = new vle::devs::ExternalEvent(
            portName);
        vle::value::Map::const_iterator it = event->getAttributes().begin();

        while (it != event->getAttributes().end()) {
            ee->putAttribute(it->first, it->second->clone());
            ++it;
        }
        return ee;
    }

    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        mPhase = IDLE;
        return vle::devs::Time::infinity;
    }

    void output(const vle::devs::Time& /* time */,
                vle::devs::ExternalEventList& output) const
    {
        for (events::const_iterator it = mEvents.begin(); it != mEvents.end();
             ++it) {
            output.addEvent(*it);
        }
    }

    vle::devs::Time timeAdvance() const
    {
        if (mPhase == IDLE) return vle::devs::Time::infinity;
        else return 0;
    }

    void internalTransition(const vle::devs::Time& /* time */)
    {
        mEvents.clear();
        mPhase = IDLE;
    }

    void externalTransition(
        const vle::devs::ExternalEventList& events,
        const vle::devs::Time& time)
    {
        vle::devs::ExternalEventList::const_iterator it = events.begin();

        while (it != events.end()) {
            ContentType type;

            if ((*it)->onPort("container")) {
                Container container(vle::value::toMapValue(
                                        (*it)->getAttributeValue("container")));

                type = container.type();
            } else {
                type = (ContentType)(
                    (*it)->getIntegerAttributeValue("type"));
            }
            std::string portName =
                (vle::fmt("%1%_%2%") % (*it)->getPortName() %
                 (type == FOOD ? "Food" : "NoFood")).str();

            // std::cout << (unsigned int)time << " - DISPATCH: "
            //           << portName << std::endl;

            mEvents.push_back(cloneExternalEvent(*it, portName));
            ++it;
        }
        mPhase = SEND;
    }

private:
    enum phase { IDLE, SEND };

    typedef std::list < vle::devs::ExternalEvent* > events;

    // state
    phase mPhase;
    events mEvents;
};

} // namespace logistics

DECLARE_NAMED_DYNAMICS(Dispatch, logistics::Dispatch);
