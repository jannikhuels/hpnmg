#pragma once

#include "Place.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

namespace hpnmg {
    class DiscretePlace : public Place {
    public:
        DiscretePlace(std::string id, unsigned long marking);
        unsigned long getMarking();

    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::base_object<Place>(*this);
            ar & this->marking;
        }
        unsigned long marking;
    };
}