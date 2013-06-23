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
        mContainerPresent =
            vle::value::toBoolean(events.get("ContainerPresent"));
        mTransportType =
            (TransportType)vle::value::toInteger(events.get("TransportType"));
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
                vle::value::toSetValue(events.get("Destinations"));

            for (unsigned int i = 0; i < values->size(); ++i) {
                mDestinationNames.push_back(
                    vle::value::toString(values->get(i)));
            }
        }

        if (mContainerPresent) {
            mMinSize = vle::value::toInteger(events.get("MinSize"));
            mMinTravelDuration =
                vle::value::toDouble(events.get("MinTravelDuration"));
            mMaxTravelDuration =
                vle::value::toDouble(events.get("MaxTravelDuration"));
        }
    }

    void generateContainers(const vle::devs::Time& time, unsigned int capacity)
    {
        unsigned int size = (mMinSize < capacity) ?
            rand().getInt(mMinSize, capacity) : capacity;

        std::cout << time << " - [" << getModelName()
                  << "] CONTAINERS GENERATE: " << size << std::endl;

        for (unsigned int i = 0; i < size; ++i) {
            std::string source =
                mDestinationNames[rand().getInt(0,
                                                mDestinationNames.size() - 1)];
            std::string destination =
                mDestinationNames[rand().getInt(0,
                                                mDestinationNames.size() - 1)];
            ContentType type = rand().getBool() ? FOOD : NOFOOD;
            vle::devs::Time exigibilityDate =
                time + rand().getDouble(mMinTravelDuration, mMaxTravelDuration);

            mContainers.add(
                new Container(mContainerID++, source, destination,
                              type, exigibilityDate));
        }
    }

    void generateTransport(const vle::devs::Time& time)
    {
        unsigned int capacity = rand().getInt(mMinCapacity, mMaxCapacity);
        std::string destination =
            mDestinationNames[rand().getInt(0, mDestinationNames.size() - 1)];
        ContentType type = rand().getBool() ? FOOD : NOFOOD;
        vle::devs::Time departureDate =
            time + rand().getDouble(mMinStayDuration, mMaxStayDuration);

        mTransport = new Transport(mTransportID++, mTransportType,
                                   capacity, destination,
                                   type, departureDate);
        if (mContainerPresent) {
            generateContainers(time, capacity);
        }
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
            ee << vle::devs::attribute("containers", mContainers.toValue());
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
            generateTransport(time);
            mPhase = SEND;
        } else if (mPhase == SEND) {
            delete mTransport;
            mTransport = 0;
            mContainers.clear();
            mPhase = IDLE;
        }
    }

private:
    enum phase { IDLE, SEND };

    // parameters
    bool mContainerPresent;
    double mMinDuration;
    double mMaxDuration;

    // transport parameters
    TransportType mTransportType;
    double mMinCapacity;
    double mMaxCapacity;
    double mMinStayDuration;
    double mMaxStayDuration;

    // container parameters
    unsigned int mMinSize;
    double mMinTravelDuration;
    double mMaxTravelDuration;

    std::vector < std::string > mDestinationNames;

    // state
    phase mPhase;
    static int mTransportID;
    Transport* mTransport;
    static int mContainerID;
    Containers mContainers;
};

int TransportGenerator::mTransportID = 0;
int TransportGenerator::mContainerID = 0;

} // namespace logistics

DECLARE_NAMED_DYNAMICS(TransportGenerator, logistics::TransportGenerator);
