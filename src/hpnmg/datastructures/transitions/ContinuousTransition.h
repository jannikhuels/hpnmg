#pragma once

#include "Transition.h"

namespace hpnmg {
    class ContinuousTransition : public Transition {

    private:
        double rate;

    public:
        ContinuousTransition(std::string id, double rate);
        double getRate();
    };
}