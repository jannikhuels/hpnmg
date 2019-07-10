#pragma once

#include "Place.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

using namespace std;
namespace hpnmg {
    class ContinuousPlace : public Place {

    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::base_object<Place>(*this);
            ar & this->capacity;
            ar & this->level;
            ar & this->infiniteCapacity;
        }

        double capacity;
        double level;
        bool infiniteCapacity;

    public:
        ContinuousPlace(string id, double capacity, double level, bool infinitecapacity);
        double getLevel();
        double getCapacity();
        bool getInfiniteCapacity();
    };
}