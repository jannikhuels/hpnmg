#pragma once

#include "Arc.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

namespace hpnmg {
    class ContinuousArc : public Arc {
    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::base_object<Arc>(*this);
            ar & this->priority;
            ar & this->share;
        }

        unsigned long priority;
        double share;

    public:
        ContinuousArc(std::string id, float weight, std::shared_ptr<Place> place, unsigned long priority, double share);
        unsigned long getPriority();
        double getShare();
    };
}