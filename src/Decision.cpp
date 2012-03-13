/**
 * @file Decision.cpp
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
#include <Transport.hpp>

namespace logistics {

class Decision : public vle::devs::Dynamics
{
public:
    Decision(const vle::devs::DynamicsInit& init,
          const vle::devs::InitEventList& events) :
        vle::devs::Dynamics(init, events)
    { }

    void removeWaitingTransport(unsigned int ID)
    {
        bool found = false;
        Transports::iterator it = mWaitingTransports.begin();

        while (not found and it != mWaitingTransports.end()) {
            if ((*it)->id() == ID) {
                found = true;
            } else {
                ++it;
            }
        }
        if (found) {
            mSelectedWaitingTransport = *it;
            mWaitingTransports.erase(it);
        }
    }

    void searchTransport(const vle::devs::Time& time)
    {
        bool found = false;
        Transports::const_iterator it = mTransports.begin();

        std::cout << time << " - [" << getModelName()
                  << "] DECISION: SEARCH TRANSPORT";

        mSelectedArrivedTransport = 0;
        while (not found and it != mTransports.end()) {
            double e = std::abs((*it)->departureDate() - time);

            if (e < 1e-5) {
                found = true;
            } else {
                ++it;
            }
        }
        if (found) {
            mSelectedArrivedTransport = *it;

            std::cout << " => " << mSelectedArrivedTransport->id()
                      << std::endl;

        } else {
            std::cout << " => NOT FOUND ---> PB !!!!!!" << std::endl;
        }
    }

    void updateSigma(const vle::devs::Time& time)
    {

        std::cout.precision(12);
        std::cout << (double)time << "SIGMA: " << mTransports.toString()
                  << std::endl;

        if (mTransports.empty()) {
            mSigma = vle::devs::Time::infinity;
        } else {
            vle::devs::Time t = vle::devs::Time::infinity;
            Transports::const_iterator it = mTransports.begin();

            while (it != mTransports.end()) {
                if ((*it)->departureDate() < t) {
                    t = (*it)->departureDate();
                }
                ++it;
            }
            if (mPhase == SEND_SEARCH or mSigma > t - time) {

                std::cout << (double)time << "SIGMA: " << t << " "
                          << time << std::endl;

                mSigma = t - time;
            }
        }
    }

    void waitContainer()
    {
        bool found = false;
        Transports::iterator it = mTransports.begin();

        while (not found and it != mTransports.end()) {
            if ((*it) == mSelectedArrivedTransport) {
                found = true;
            } else {
                ++it;
            }
        }
        if (found) {
            mWaitingTransports.push_back(*it);
            mTransports.erase(it);
            mSelectedArrivedTransport = 0;
        }
    }

/*  - - - - - - - - - - - - - --ooOoo-- - - - - - - - - - - -  */

    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        mPhase = IDLE;
        mSigma = vle::devs::Time::infinity;
        return vle::devs::Time::infinity;
    }

    void output(const vle::devs::Time& time,
                vle::devs::ExternalEventList& output) const
    {
        if (mPhase == SEND_SEARCH) {
            vle::devs::ExternalEvent* ee =
                new vle::devs::ExternalEvent("search");

            std::cout << time << " - [" << getModelName()
                      << "] DECISION: SEARCH -> "
                      << mSelectedArrivedTransport->toString() << std::endl;

            ee << vle::devs::attribute("type",
                                       mSelectedArrivedTransport->type());
            ee << vle::devs::attribute("transport",
                                       mSelectedArrivedTransport->toValue());
            output.addEvent(ee);
        } else if (mPhase == SEND_TAKE) {
            vle::devs::ExternalEvent* ee =
                new vle::devs::ExternalEvent("take");

            std::cout << time << " - [" << getModelName()
                      << "] DECISION: TAKE -> "
                      << mSelectedWaitingTransport->toString() << std::endl;

            ee << vle::devs::attribute("type",
                                       mSelectedWaitingTransport->type());
            ee << vle::devs::attribute("transport",
                                       mSelectedWaitingTransport->toValue());
            output.addEvent(ee);
        }
    }

    vle::devs::Time timeAdvance() const
    {
        if (mPhase == SEND_TAKE or mPhase == SEND_SEARCH) {
            return 0;
        } else {
            return mSigma;
        }
    }

    void internalTransition(const vle::devs::Time& time)
    {

        std::cout.precision(12);
        std::cout << time << " - [" << getModelName()
                  << "] internalTransition: " << mPhase << std::endl;

        if (mPhase == IDLE) {
            searchTransport(time);
            mPhase = SEND_SEARCH;
        } else if (mPhase == SEND_SEARCH) {
            waitContainer();
            updateSigma(time);
            mPhase = IDLE;
        } else if (mPhase == SEND_TAKE) {
            delete mSelectedWaitingTransport;
            mSelectedWaitingTransport = 0;
            mPhase = IDLE;
        }
    }

    void externalTransition(
        const vle::devs::ExternalEventList& events, const vle::devs::Time& time)
    {
        vle::devs::ExternalEventList::const_iterator it = events.begin();

        std::cout.precision(12);
        std::cout << time << " - [" << getModelName()
                  << "] externalTransition: " << mPhase << std::endl;

        while (it != events.end()) {
            if ((*it)->onPort("transport")) {
                Transport* transport = new Transport(
                    vle::value::toMapValue(
                        (*it)->getAttributeValue("transport")));

                std::cout << time << " - [" << getModelName()
                          << "] DECISION TRANSPORT: " << transport->toString()
                          << " => " << mPhase << std::endl;

                transport->arrived(time);
                mTransports.push_back(transport);
            } else if ((*it)->onPort("found")) {
                unsigned int transportID =
                    (*it)->getIntegerAttributeValue("id_transport");

                std::cout << time << " - [" << getModelName()
                          << "] DECISION FOUND: transport -> " << transportID
                          << std::endl;

                mContainerID = (*it)->getIntegerAttributeValue("id_container");

                std::cout << time << " - [" << getModelName()
                          << "] DECISION FOUND: container -> " << mContainerID
                          << std::endl;

                removeWaitingTransport(transportID);
                mPhase = SEND_TAKE;
            }
            ++it;
        }
        updateSigma(time);

        // std::cout << time << " - [" << getModelName()
        //           << "] DECISION TRANSPORT: sigma => "
        //           << mSigma << std::endl;

    }

    vle::value::Value* observation(
        const vle::devs::ObservationEvent& event) const
    {
        if (event.onPort("size")) {
            return vle::value::Integer::create(mTransports.size());
        } else if (event.onPort("wait")) {
            return vle::value::Integer::create(mWaitingTransports.size());
        } else {
            return 0;
        }
    }

private:
    enum phase { IDLE, SEND_SEARCH, SEND_TAKE };

    // state
    phase mPhase;
    vle::devs::Time mSigma;
    Transports mTransports;
    Transport* mSelectedArrivedTransport;
    Transport* mSelectedWaitingTransport;
    Transports mWaitingTransports;
    unsigned int mContainerID;
};

} // namespace logistics

DECLARE_NAMED_DYNAMICS(Decision, logistics::Decision);
