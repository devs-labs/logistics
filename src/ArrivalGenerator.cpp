/**
 * @file ArrivalGenerator.cpp
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
#include <Container.hpp>

namespace logistics {

class ArrivalGenerator : public vle::devs::Dynamics
{
public:
    ArrivalGenerator(const vle::devs::DynamicsInit& init,
                     const vle::devs::InitEventList& events) :
        vle::devs::Dynamics(init, events)
    {
        mName = vle::value::toString(events.get("Name"));

        mMinCapacity = vle::value::toInteger(events.get("MinCapacity"));
        mMaxCapacity = vle::value::toInteger(events.get("MaxCapacity"));
        mMinDuration = vle::value::toDouble(events.get("MinDuration"));
        mMaxDuration = vle::value::toDouble(events.get("MaxDuration"));
        mMinSize = vle::value::toInteger(events.get("MinSize"));
        mMaxSize = vle::value::toInteger(events.get("MaxSize"));
        mMinTravelDuration =
            vle::value::toInteger(events.get("MinTravelDuration"));
        mMaxTravelDuration =
            vle::value::toInteger(events.get("MaxTravelDuration"));

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
        unsigned int size = rand().getInt(mMinSize, mMaxSize);

        std::cout << time << " - [" << mName
                  << "] CONTAINERS GENERATE: " << size << std::endl;

        for (unsigned int i = 0; i < size; ++i) {
            double capacity = rand().getDouble(mMinCapacity, mMaxCapacity);
            std::string source = mNames[rand().getInt(0, mNames.size() - 1)];
            std::string destination =
                mNames[rand().getInt(0, mNames.size() - 1)];
            ContentType type = rand().getBool() ? FOOD : NOFOOD;
            vle::devs::Time exigibilityDate =
                time + rand().getDouble(mMinTravelDuration, mMaxTravelDuration);

            mContainers.add(
                new Container(mContainerID++,
                              capacity, source, destination,
                              type, exigibilityDate));
        }
    }

    vle::devs::Time nextDate() const
    {
        return rand().getDouble(mMinDuration, mMaxDuration);
    }

    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        mPhase = IDLE;
        return nextDate();
    }

    void output(const vle::devs::Time& /* time */,
                vle::devs::ExternalEventList& output) const
    {
        if (mPhase == SEND) {
            vle::devs::ExternalEvent* ee = new vle::devs::ExternalEvent("out");

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
            generate(time);
            mPhase = SEND;
        } else if (mPhase == SEND) {
            mContainers.clear();
            mPhase = IDLE;
        }
    }

private:
    enum phase { IDLE, SEND };

    // parameters
    std::string mName;
    double mMinCapacity;
    double mMaxCapacity;
    double mMinDuration;
    double mMaxDuration;
    double mMinSize;
    double mMaxSize;
    double mMinTravelDuration;
    double mMaxTravelDuration;
    std::vector < std::string > mNames;

    // state
    phase mPhase;
    static int mContainerID;
    Containers mContainers;
};

int ArrivalGenerator::mContainerID = 0;

} // namespace logistics

DECLARE_NAMED_DYNAMICS(ArrivalGenerator, logistics::ArrivalGenerator);
