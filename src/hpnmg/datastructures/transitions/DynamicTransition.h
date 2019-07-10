#pragma once

#include "ContinuousTransition.h"

#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace hpnmg {
    class DynamicTransition : public ContinuousTransition {

    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::base_object<ContinuousTransition>(*this);
            ar & this->factor;
            ar & this->constant;
            ar & this->transitions;
            ar & this->transitionFactors;
            ar & this->parameter;
        }

        double factor;
        double constant;
        std::vector<shared_ptr<ContinuousTransition>> transitions;
        std::vector<double> transitionFactors;
        double parameter;

    public:
        DynamicTransition(std::string id,
                          double factor,
                          double constant,
                          std::vector<shared_ptr<ContinuousTransition>> transitions,
                          std::vector<double> transitionFactors,
                          double parameter
            );
        double getRate();
        double getFactor();
        double getConstant();
        std::vector<shared_ptr<ContinuousTransition>> getTransitions();
        std::vector<double> getTransitionFactors();
        double getParameter();
    };
}