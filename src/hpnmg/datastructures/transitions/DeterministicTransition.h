#pragma once

#include "Transition.h"

namespace hpnmg {
    class DeterministicTransition : public Transition {

    private:
        unsigned long priority;
        float weight;
        float discTime;

    public:
        DeterministicTransition(std::string id, unsigned long priority, float weight, float discTime);
    };
}