#pragma once

#include <vector>
#include "ContinuousTransition.h"

namespace hpnmg {
    class DynamicTransition : public ContinuousTransition {

    private:
        int factor;
        double constant;
        std::vector<ContinuousTransition> transitions;
        std::vector<int> transitionFactors;
        int parameter;



    public:
        DynamicTransition(std::string id,
                          int factor,
                          double constant,
                          std::vector<ContinuousTransition> transitions,
                          std::vector<int> transitionFactors,
                          int parameter
            );
        double getRate();
    };
}