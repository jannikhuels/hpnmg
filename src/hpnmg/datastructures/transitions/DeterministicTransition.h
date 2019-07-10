#pragma once

#include "Transition.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

namespace hpnmg {
    class DeterministicTransition : public Transition {

    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::base_object<Transition>(*this);
            ar & this->priority;
            ar & this->weight;
            ar & this->discTime;
        }

        unsigned long priority;
        double weight;
        double discTime;

    public:
        DeterministicTransition(std::string id, unsigned long priority, double weight, double discTime);
        double getDiscTime();
        double getWeight();
        double getPriority();
        void setDiscTime(double discTime);
    };
}