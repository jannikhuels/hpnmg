#pragma once

#include "Transition.h"

namespace hpnmg {
    class DeterministicTransition : public Transition {

    private:
        unsigned long priority;
        double weight;
        double discTime;

    public:
        DeterministicTransition(std::string id, unsigned long priority, double weight, double discTime);
        double getDiscTime();
        double getWeight();
    };
}