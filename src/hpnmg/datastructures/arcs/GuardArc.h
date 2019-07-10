#pragma once

#include "Arc.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

namespace hpnmg {
    class GuardArc : public Arc {
    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::base_object<Arc>(*this);
            ar & this->isInhibitor;
        }

        bool isInhibitor;

    public:
        GuardArc(std::string id, float weight, std::shared_ptr<Place> place, bool isInhibitor);
        bool getIsInhibitor();
    };
}