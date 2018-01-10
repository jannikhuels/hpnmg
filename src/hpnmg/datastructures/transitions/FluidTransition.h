#pragma once

#include "Transition.h"

namespace hpnmg {
    class FluidTransition : public Transition {

    private:
        float rate;

    public:
        FluidTransition(std::string id, float rate);
    };
}