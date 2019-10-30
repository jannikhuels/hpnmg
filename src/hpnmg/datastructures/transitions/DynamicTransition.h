#pragma once

#include "ContinuousTransition.h"

#include <memory>
#include <vector>

namespace hpnmg {
    class DynamicTransition : public ContinuousTransition {

    private:
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