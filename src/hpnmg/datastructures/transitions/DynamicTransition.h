#pragma once

#include <vector>
#include "ContinuousTransition.h"

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
    };
}