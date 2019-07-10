#pragma once

#include "../places/Place.h"

#include <memory>
#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace hpnmg {
    class Arc {
    public:
        std::string id;
        float weight;
        std::shared_ptr<Place> place;

        Arc(std::string id, float weight, std::shared_ptr<Place> place);

    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & this->id;
            ar & this->weight;
            ar & this->place;
        }
    };
}
