#pragma once

#include "Transition.h"

namespace hpnmg {
    class ImmediateTransition : public Transition {

    private:
        unsigned long priority;
        float weight;

    public:
        ImmediateTransition(std::string id, unsigned long priority, float weight);
        unsigned long getPriority();
        float getWeight();
    };
}
