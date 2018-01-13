#pragma once

#include <map>
#include "Transition.h"

namespace hpnmg {
    class GeneralTransition : public Transition {

    private:
        unsigned long priority;
        float weight;
        std::string cdf;
        std::map<std::string, float> parameter;
        // todo: policy

    public:
        GeneralTransition(std::string id, unsigned long priority, float weight, std::string cdf,
                          std::map<std::string, float> parameter);
    };
}