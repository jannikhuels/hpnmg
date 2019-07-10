#pragma once

#include "Transition.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

namespace hpnmg {
    class ImmediateTransition : public Transition {

    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::base_object<Transition>(*this);
            ar & this->priority;
            ar & this->weight;
        }

        unsigned long priority;
        float weight;

    public:
        ImmediateTransition(std::string id, unsigned long priority, float weight);
        unsigned long getPriority();
        float getWeight();
    };
}
