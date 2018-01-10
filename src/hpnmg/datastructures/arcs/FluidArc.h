#pragma once

#include "Arc.h"

namespace hpnmg {
    class FluidArc : public Arc {
    private:
        unsigned long priority;
        float share;

    public:
        FluidArc(std::string id, float weight, Transition transition, Place place, bool isInputArc,
                 unsigned long priority, float share);
    };
}