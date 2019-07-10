#pragma once

#include "Transition.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>

namespace hpnmg {
    class ContinuousTransition : public Transition {

    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::base_object<Transition>(*this);
            ar & this->rate;
            ar & this->currentRate;
        }

        double rate;

    public:
        double currentRate;
        ContinuousTransition(std::string id, double rate);

        virtual double getRate();

        double getCurrentRate();
        void setCurrentRate(double newRate);
    };
}