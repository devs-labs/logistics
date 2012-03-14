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

class Transit : public vle::devs::Dynamics
{
public:
    Transit(const vle::devs::DynamicsInit& init,
            const vle::devs::InitEventList& events) :
        vle::devs::Dynamics(init, events)
    {
    }

    void removeReadyTransports()
    {
        ReadyTransports::iterator it = mReadyTransports.begin();

        while (it != mReadyTransports.end()) {
            {
                LoadingTransports::iterator itc = mLoadingTransports.find(*it);
                const Containers& containers = itc->second;
                Containers::const_iterator itcc = containers.begin();

                while (itcc != containers.end()) {
                    removeSelectedContainer(*itcc);
                    ++itcc;
                }
                mLoadingTransports.erase(itc);
            }
            {
                bool found = false;
                OrderedTransportList::iterator itc = mWaitingTransports.begin();

                while (not found and itc != mWaitingTransports.end()) {
                    if ((*itc)->id() == *it) {
                        mWaitingTransports.erase(itc);
                        found = true;
                    } else {
                        ++itc;
                    }
                }
            }
            ++it;
        }
        mReadyTransports.clear();
    }

    void removeSelectedContainer(Container* container)
    {
        bool found = false;
        Containers::iterator it = mWaitingContainers.begin();

        while (not found and it != mWaitingContainers.end()) {
            if ((*it) == container) {
                found = true;
            } else {
                ++it;
            }
        }
        if (found) {
            delete *it;
            mWaitingContainers.erase(it);
        }
    }

    void loadContainer(Transport* transport)
    {
        vle::devs::Time time = vle::devs::Time::infinity;
        Containers::iterator it = mWaitingContainers.begin();
        Containers::iterator itc;
        Container* selectedContainer = 0;

        while (it != mWaitingContainers.end()) {
            if ((*it)->exigibilityDate() < time) {
                time = (*it)->exigibilityDate();
                itc = it;
                selectedContainer = *it;
            }
            ++it;
        }
        if (selectedContainer != 0) {
            mLoadingTransports[transport->id()].push_back(selectedContainer);
            mWaitingContainers.erase(itc);
        }
    }

    bool loadContainers()
    {
        bool loaded = false;
        bool stop = false;
        OrderedTransportList::const_iterator it =
            mWaitingTransports.begin();

        while (not stop and it != mWaitingTransports.end()) {
            LoadingTransports::const_iterator itt =
                mLoadingTransports.find((*it)->id());

            if (itt == mLoadingTransports.end()) {
                mLoadingTransports[(*it)->id()] = Containers();
                itt = mLoadingTransports.find((*it)->id());
            }
            if ((int)itt->second.size() < (*it)->capacity()) {
                loadContainer(*it);
                loaded = (int)itt->second.size() == (*it)->capacity();
                stop = mWaitingContainers.empty();
            } else {
                ++it;
            }
        }
        return loaded;
    }

/*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */

    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        return vle::devs::Time::infinity;
    }

    void output(const vle::devs::Time& time,
                vle::devs::ExternalEventList& output) const
    {
        if (mPhase == LOADED) {
            OrderedTransportList::const_iterator it =
                mWaitingTransports.begin();

            std::cout << time << " - [" << getModelName()
                      << "] TRANSIT LOADED: ";

            while (it != mWaitingTransports.end()) {
                LoadingTransports::const_iterator itt =
                    mLoadingTransports.find((*it)->id());

                if (itt != mLoadingTransports.end() and
                    (int)itt->second.size() == (*it)->capacity()) {
                    vle::devs::ExternalEvent* ee =
                        new vle::devs::ExternalEvent("loaded");

                    std::cout << (*it)->id() << " ";

                    ee << vle::devs::attribute("id", (int)(*it)->id());
                    output.addEvent(ee);
                }
                ++it;
            }

            std::cout << std::endl;

        } else if (mPhase == OUT) {
            ReadyTransports::const_iterator it = mReadyTransports.begin();

            std::cout << time << " - [" << getModelName()
                      << "] TRANSIT OUT: { ";

            while (it != mReadyTransports.end()) {
                vle::devs::ExternalEvent* ee =
                    new vle::devs::ExternalEvent("out");
                LoadingTransports::const_iterator itc =
                    mLoadingTransports.find(*it);
                Transport* transport = mWaitingTransports.find(*it);

                std::cout << *it << " ";

                ee << vle::devs::attribute("transport", transport->toValue());
                ee << vle::devs::attribute("containers", itc->second.toValue());
                output.addEvent(ee);
                ++it;
            }

            std::cout << "}" << std::endl;

        }
    }

    vle::devs::Time timeAdvance() const
    {
        if (mPhase == LOADED or mPhase == OUT) {
            return 0;
        } else {
            return vle::devs::Time::infinity;
        }
    }

    void internalTransition(const vle::devs::Time& /* time */)
    {
        if (mPhase == OUT) {
            removeReadyTransports();
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
                          << "] TRANSIT CONTAINER: " << container->toString()
                          << std::endl;

                container->arrived(time);
                mWaitingContainers.push_back(container);
            } else if ((*it)->onPort("load")) {
                Transport* transport = new Transport(
                    vle::value::toMapValue(
                        (*it)->getAttributeValue("transport")));

                std::cout << time << " - [" << getModelName()
                          << "] TRANSIT LOAD: " << transport->id()
                          << std::endl;

                mWaitingTransports.push_back(transport);

                std::cout << time << " - [" << getModelName()
                          << "] TRANSIT LOAD: wait = "
                          << mWaitingTransports.size() << std::endl;

            } else if ((*it)->onPort("depart")) {
                TransportID transportID =
                    (TransportID)(*it)->getIntegerAttributeValue("id");

                std::cout << time << " - [" << getModelName()
                          << "] TRANSIT DEPART: " << transportID
                          << std::endl;

                mReadyTransports.push_back(transportID);
                mPhase = OUT;
            }
            ++it;
        }
        if (not mWaitingTransports.empty() and mWaitingContainers.size() > 0) {

            std::cout << time << " - [" << getModelName()
                      << "] TRANSIT LOADING";

            if (loadContainers()) {

                std::cout << " ==> loaded" << std::endl;

                mPhase = LOADED;
            }

            std::cout  << std::endl;

        }
    }

    vle::value::Value* observation(
        const vle::devs::ObservationEvent& event) const
    {
        if (event.onPort("size")) {
            return vle::value::Integer::create(mWaitingContainers.size());
        } else if (event.onPort("waiting")) {
            return vle::value::Integer::create(mWaitingTransports.size());
        } else if (event.onPort("time-in-transit")) {
            double t = 0;
            Containers::const_iterator it = mWaitingContainers.begin();

            while (it != mWaitingContainers.end()) {
                double e = event.getTime() - (*it)->arrivalDate();

                if (e > 0 ) {
                    t += e;
                }
                ++it;
            }
            if (mWaitingContainers.empty()) {
                return vle::value::Double::create(0);
            } else {
                return vle::value::Double::create(
                    t / mWaitingContainers.size());
            }
        } else if (event.onPort("transport-lateness")) {
            double t = 0;
            OrderedTransportList::const_iterator it =
                mWaitingTransports.begin();

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
    enum phase { IDLE, LOADED, OUT };

    // state
    phase mPhase;

    Containers mWaitingContainers;
    OrderedTransportList mWaitingTransports;
    LoadingTransports mLoadingTransports;
    ReadyTransports mReadyTransports;
};

} // namespace logistics

DECLARE_NAMED_DYNAMICS(Transit, logistics::Transit);
