/**
 * @file Transit.cpp
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
#include <Transport.hpp>

namespace logistics {

typedef std::list < Transport* > TransportList;

class Transit : public vle::devs::Dynamics
{
public:
    Transit(const vle::devs::DynamicsInit& init,
            const vle::devs::InitEventList& events) :
        vle::devs::Dynamics(init, events)
    {
    }

    void removeSelectedContainer()
    {
        bool found = false;
        Containers::iterator it = mContainers.begin();

        while (not found and it != mContainers.end()) {
            if ((*it) == mSelectedContainer) {
                found = true;
            } else {
                ++it;
            }
        }
        if (found) {
            delete *it;
            mContainers.erase(it);
            mSelectedContainer = 0;
        }
    }

    bool searchContainer()
    {
        vle::devs::Time time = vle::devs::Time::infinity;
        Containers::const_iterator it = mContainers.begin();

        std::cout <<"[" << getModelName() << "] DECISION: SEARCH CONTAINER";

        mSelectedContainer = 0;
        while (it != mContainers.end()) {
            if ((*it)->exigibilityDate() < time) {
                time = (*it)->exigibilityDate();
                mSelectedContainer = *it;
            } else {
                ++it;
            }
        }
        if (mSelectedContainer != 0) {

            std::cout << " => " << mSelectedContainer->id() << std::endl;

            return true;
        } else {

            std::cout << " => NOT FOUND" << std::endl;

            return false;
        }
    }

    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        return vle::devs::Time::infinity;
    }

    void output(const vle::devs::Time& /* time */,
                vle::devs::ExternalEventList& output) const
    {
        if (mPhase == FOUND) {
            vle::devs::ExternalEvent* ee =
                new vle::devs::ExternalEvent("found");
            Transport* transport = *mWaitingTransports.begin();

            ee << vle::devs::attribute("id_transport",
                                       (int)transport->id());
            ee << vle::devs::attribute("id_container",
                                       (int)mSelectedContainer->id());
            output.addEvent(ee);
        }
    }

    vle::devs::Time timeAdvance() const
    {
        if (mPhase == FOUND) {
            return 0;
        } else {
            return vle::devs::Time::infinity;
        }
    }

    void internalTransition(const vle::devs::Time& /* time */)
    {
        if (mPhase == FOUND) {
            mWaitingTransports.pop_front();
        }
        mPhase = IDLE;
    }

    void externalTransition(
        const vle::devs::ExternalEventList& events, const vle::devs::Time& time)
    {
        vle::devs::ExternalEventList::const_iterator it = events.begin();

        while (it != events.end()) {
            if ((*it)->onPort("container")) {
                Container* container = new Container(
                    vle::value::toMapValue(
                        (*it)->getAttributeValue("container")));

                std::cout << time << " - [" << getModelName()
                          << " ] TRANSIT CONTAINER: " << container->toString()
                          << std::endl;

                container->arrived(time);
                mContainers.push_back(container);
            } else if ((*it)->onPort("search")) {
                Transport* transport = new Transport(
                    vle::value::toMapValue(
                        (*it)->getAttributeValue("transport")));

                std::cout << time << " - [" << getModelName()
                          << " ] TRANSIT SEARCH: " << transport->id()
                          << std::endl;

                mWaitingTransports.push_back(transport);

                std::cout << time << " - [" << getModelName()
                          << " ] TRANSIT SEARCH: wait = "
                          << mWaitingTransports.size() << std::endl;

            } else if ((*it)->onPort("take")) {
                Transport* transport = new Transport(
                    vle::value::toMapValue(
                        (*it)->getAttributeValue("transport")));

                std::cout << time << " - [" << getModelName()
                          << " ] TRANSIT TAKE: " << transport->id()
                          << std::endl;

                removeSelectedContainer();
            }
            ++it;
        }
        if (not mWaitingTransports.empty() and searchContainer()) {
            mPhase = FOUND;
        }
    }

    vle::value::Value* observation(
        const vle::devs::ObservationEvent& event) const
    {
        if (event.onPort("size")) {
            return vle::value::Integer::create(mContainers.size());
        } else if (event.onPort("waiting")) {
            return vle::value::Integer::create(mWaitingTransports.size());
        } else if (event.onPort("time-in-transit")) {
            double t = 0;
            Containers::const_iterator it = mContainers.begin();

            while (it != mContainers.end()) {
                double e = event.getTime() - (*it)->arrivalDate();

                if (e > 0 ) {
                    t += e;
                }
                ++it;
            }
            if (mContainers.empty()) {
                return vle::value::Double::create(0);
            } else {
                return vle::value::Double::create(t / mContainers.size());
            }
        } else if (event.onPort("wait-transport-time")) {
            double t = 0;
            TransportList::const_iterator it = mWaitingTransports.begin();

            while (it != mWaitingTransports.end()) {
                double e = event.getTime() - (*it)->departureDate();

                if (e > 0 ) {
                    t += e;
                }
                ++it;
            }
            if (mWaitingTransports.empty()) {
                return vle::value::Double::create(0);
            } else {
                return vle::value::Double::create(
                    t / mWaitingTransports.size());
            }
        } else {
            return 0;
        }
    }

private:
    enum phase { IDLE, FOUND };

    // state
    phase mPhase;
    Containers mContainers;
    Container* mSelectedContainer;
    TransportList mWaitingTransports;
};

} // namespace logistics

DECLARE_NAMED_DYNAMICS(Transit, logistics::Transit);
