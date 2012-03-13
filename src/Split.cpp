/**
 * @file Split.cpp
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

namespace logistics {

class Split : public vle::devs::Dynamics
{
public:
    Split(const vle::devs::DynamicsInit& init,
          const vle::devs::InitEventList& events) :
        vle::devs::Dynamics(init, events)
    {
    }

    vle::devs::Time init(const vle::devs::Time& /* time */)
    {
        return vle::devs::Time::infinity;
    }

    void output(const vle::devs::Time& /* time */,
                vle::devs::ExternalEventList& output) const
    {
        if (mPhase == SEND) {
            for (std::vector < Containers* >::const_iterator it =
                     mContainersList.begin(); it != mContainersList.end();
                 ++it) {
                for (Containers::const_iterator itc = (*it)->begin();
                     itc != (*it)->end(); ++itc) {
                    vle::devs::ExternalEvent* ee =
                        new vle::devs::ExternalEvent("out");

                    ee << vle::devs::attribute("container",
                                               (*itc)->toValue());
                    output.addEvent(ee);
                }
            }
        }
    }

    vle::devs::Time timeAdvance() const
    {
        if (mPhase == SEND) {
            return 0;
        } else {
            return vle::devs::Time::infinity;
        }
    }

    void internalTransition(const vle::devs::Time& /* time */)
    {
        mContainersList.clear();
        mPhase = IDLE;
    }

    void externalTransition(
        const vle::devs::ExternalEventList& events, const vle::devs::Time& time)
    {
        vle::devs::ExternalEventList::const_iterator it = events.begin();

        while (it != events.end()) {
            Containers* containers = new Containers(
                vle::value::toSetValue(
                    (*it)->getAttributeValue("containers")));

            std::cout << (unsigned int)time << " - [" << getModelName()
                      << "] SPLIT: " << containers->toString() << std::endl;

            containers->arrived(time);
            mContainersList.push_back(containers);
            ++it;
        }
        mPhase = SEND;
    }

private:
    enum phase { IDLE, SEND };

    // state
    phase mPhase;
    std::vector < Containers* > mContainersList;
};

} // namespace logistics

DECLARE_NAMED_DYNAMICS(Split, logistics::Split);
