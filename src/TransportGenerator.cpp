/**
 * @file TransportGenerator.cpp
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
#include <vle/utils/Rand.hpp>
#include <Transport.hpp>

namespace logistics {

class TransportGenerator : public vle::devs::Dynamics
{
public:
    TransportGenerator(const vle::devs::DynamicsInit& init,
                     const vle::devs::InitEventList& events) :
        vle::devs::Dynamics(init, events)
    {
        mMinCapacity = vle::value::toInteger(events.get("MinCapacity"));
        mMaxCapacity = vle::value::toInteger(events.get("MaxCapacity"));
        mMinDuration = vle::value::toDouble(events.get("MinDuration"));
        mMaxDuration = vle::value::toDouble(events.get("MaxDuration"));
        mMinStayDuration =
            vle::value::toDouble(events.get("MinStayDuration"));
        mMaxStayDuration =
            vle::value::toDouble(events.get("MaxStayDuration"));

        {
            const vle::value::Set* values =
                vle::value::toSetValue(events.get("Names"));

            for (unsigned int i = 0; i < values->size(); ++i) {
                mNames.push_back(vle::value::toString(values->get(i)));
            }
        }
    }

    void generate(const vle::devs::Time& time)
    {
        unsigned int capacity = rand().getInt(mMinCapacity, mMaxCapacity);
        std::string destination =
            mNames[rand().getInt(0, mNames.size() - 1)];
        ContentType type = rand().getBool() ? FOOD : NOFOOD;
        vle::devs::Time departureDate =
            time + rand().getDouble(mMinStayDuration, mMaxStayDuration);

        mTransport = new Transport(mTransportID++,
                                   capacity, destination,
                                   type, departureDate);
    }

    vle::devs::Time nextDate() const
    {
        return rand().getDouble(mMinDuration, mMaxDuration);
    }

    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        mTransport = 0;
        mPhase = IDLE;
        return nextDate();
    }

    void output(const vle::devs::Time& /* time */,
                vle::devs::ExternalEventList& output) const
    {
        if (mPhase == SEND) {
            vle::devs::ExternalEvent* ee = new vle::devs::ExternalEvent("out");

            ee << vle::devs::attribute("transport", mTransport->toValue());
            output.addEvent(ee);
        }
    }

    vle::devs::Time timeAdvance() const
    {
        if (mPhase == IDLE) {
            return nextDate();
        } else if (mPhase == SEND) {
            return 0;
        }
        return vle::devs::Time::infinity;
    }

    void internalTransition(const vle::devs::Time& time)
    {
        if (mPhase == IDLE) {
            generate(time);
            mPhase = SEND;
        } else if (mPhase == SEND) {
            delete mTransport;
            mTransport = 0;
            mPhase = IDLE;
        }
    }

private:
    enum phase { IDLE, SEND };

    // parameters
    double mMinCapacity;
    double mMaxCapacity;
    double mMinDuration;
    double mMaxDuration;
    double mMinStayDuration;
    double mMaxStayDuration;
    std::vector < std::string > mNames;

    // state
    phase mPhase;
    static int mTransportID;
    Transport* mTransport;
};

int TransportGenerator::mTransportID = 0;

} // namespace logistics

DECLARE_NAMED_DYNAMICS(TransportGenerator, logistics::TransportGenerator);
