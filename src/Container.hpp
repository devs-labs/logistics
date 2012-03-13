/**
 * @file Container.hpp
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

#ifndef CONTAINER_HPP
#define CONTAINER_HPP 1

#include <vle/value/Map.hpp>
#include <vle/devs/Time.hpp>

using namespace vle::devs;
using namespace vle::value;

namespace logistics {

enum ContentType { FOOD, NOFOOD };

typedef std::vector < std::string > path_t;

class Container
{
public:
    Container(int id, double capacity, const std::string& source,
              const std::string& destination, ContentType contentType,
              Time exigibilityDate) :
        mID(id), mCapacity(capacity), mSource(source),
        mDestination(destination), mContentType(contentType),
        mExigibilityDate(exigibilityDate)
    { }

    Container(const Map& value)
    {
        mID = toInteger(value.get("Id"));
        mCapacity = toDouble(value.get("Capacity"));
        mSource = vle::value::toString(value.get("Source"));
        mDestination = vle::value::toString(value.get("Destination"));
        mContentType =
            (ContentType)toInteger(value.get("ContentType"));
        mExigibilityDate = (Time)toDouble(value.get("ExigibilityDate"));
        {
            const Set* path = toSetValue(value.get("Path"));

            for (unsigned int i = 0; i < path->size(); ++i) {
                mPath.push_back(vle::value::toString(path->get(i)));
            }
        }
    }

    virtual ~Container()
    { }

    void arrived(const Time& time)
    { mArrivalDate = time; }

    Time arrivalDate() const
    { return mArrivalDate; }

    double capacity() const
    { return mCapacity; }

    std::string destination() const
    { return mDestination; }

    Time exigibilityDate() const
    { return mExigibilityDate; }

    unsigned int id() const
    { return mID; }

    std::string toString() const
    {
        std::ostringstream str;

        str << "Container[ " << mID << " " << mCapacity << " " << mSource
            << " " << mDestination
            << " " << ((mContentType == FOOD) ? "FOOD" : "NOFOOD")
            << " " << mExigibilityDate << " < ";
        for (path_t::const_iterator it = mPath.begin();
             it != mPath.end(); ++it) {
            str << *it;
        }
        str << "> ] ";
        return str.str();
    }

    Value* toValue() const
    {
        Map* value = new Map;

        value->addInt("Id", mID);
        value->addDouble("Capacity", mCapacity);
        value->addString("Source", mSource);
        value->addString("Destination", mDestination);
        value->addInt("ContentType", mContentType);
        value->addDouble("ExigibilityDate", mExigibilityDate.getValue());
        {
            Set* path = new Set;

            for (path_t::const_iterator it = mPath.begin();
                 it != mPath.end(); ++it) {
                path->addString(*it);
            }
            value->add("Path", path);
        }
        return value;
    }

    ContentType type() const
    { return mContentType; }

private:
    int mID;
    double mCapacity;
    std::string mSource;
    std::string mDestination;
    ContentType mContentType;
    Time mExigibilityDate;
    Time mArrivalDate;
    path_t mPath;
};

class Containers : public std::vector < Container* >
{
public:
    Containers()
    { }

    Containers(const Set& value)
    {
        for (unsigned int i = 0; i < value.size(); ++i) {
            add(new Container(*toMapValue(value.get(i))));
        }
    }

    virtual ~Containers()
    {
        for (const_iterator it = begin(); it != end(); ++it) {
            delete *it;
        }
    }

    void add(Container* container)
    {
        push_back(container);
    }

    void arrived(const Time& time)
    {
        for (const_iterator it = begin(); it != end(); ++it) {
            (*it)->arrived(time);
        }
    }

    void clear()
    {
        for (const_iterator it = begin(); it != end(); ++it) {
            delete *it;
        }
        std::vector < Container* >::clear();
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

    Value* toValue() const
    {
        Set* value = new Set;

        for (const_iterator it = begin(); it != end(); ++it) {
            value->add((*it)->toValue());
        }
        return value;
    }
};

} // namespace logistics

#endif
