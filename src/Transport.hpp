/**
 * @file Transport.hpp
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

#ifndef TRANSPORT_HPP
#define TRANSPORT_HPP 1

#include <vle/value/Map.hpp>
#include <vle/devs/Time.hpp>
#include <Container.hpp>

using namespace vle::devs;
using namespace vle::value;

namespace logistics {

class Transport
{
public:
    Transport(int id, double capacity, const std::string& destination,
              ContentType contentType, Time departureDate) :
        mID(id), mCapacity(capacity), mDestination(destination),
        mContentType(contentType), mDepartureDate(departureDate)
    { }

    Transport(const Map& value)
    {
        mID = toInteger(value.get("Id"));
        mCapacity = toDouble(value.get("Capacity"));
        mDestination = vle::value::toString(value.get("Destination"));
        mContentType =
            (ContentType)toInteger(value.get("ContentType"));
        mDepartureDate = (Time)toDouble(value.get("DepartureDate"));
    }

    virtual ~Transport()
    { }

    void arrived(const Time& time)
    {
        mArrivalDate = time;
    }

    std::string toString() const
    {
        std::ostringstream str;

        str << "Transport[ " << mID << " " << mCapacity
            << " " << mDestination
            << " " << ((mContentType == FOOD) ? "FOOD" : "NOFOOD")
            << " " << mDepartureDate << " ] ";
        return str.str();
    }

    Value* toValue() const
    {
        Map* value = new Map;

        value->addInt("Id", mID);
        value->addDouble("Capacity", mCapacity);
        value->addString("Destination", mDestination);
        value->addInt("ContentType", mContentType);
        value->addDouble("DepartureDate", mDepartureDate.getValue());
        return value;
    }

    int capacity() const
    { return mCapacity; }

    Time departureDate() const
    { return mDepartureDate; }

    std::string destination() const
    { return mDestination; }

    unsigned int id() const
    { return mID; }

    ContentType type() const
    { return mContentType; }

private:
    int mID;
    double mCapacity;
    std::string mDestination;
    ContentType mContentType;
    Time mDepartureDate;
    Time mArrivalDate;
};

class Transports : public std::vector < Transport* >
{
public:
    virtual ~Transports()
    {
        for (const_iterator it = begin(); it != end(); ++it) {
            delete *it;
        }
    }

    void clear()
    {
        for (const_iterator it = begin(); it != end(); ++it) {
            delete *it;
        }
        std::vector < Transport* >::clear();
    }

    std::string toString() const
    {
        std::string str = "{ ";

        for (const_iterator it = begin(); it != end(); ++it) {
            str += (*it)->toString();
        }
        str += "}";
        return str;
    }
};

} // namespace logistics

#endif
