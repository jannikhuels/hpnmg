#pragma once

#include "Arc.h"

namespace hpnmg {
    class FluidArc : public Arc {
    private:
        unsigned long priority;
        float share;

    public:
        FluidArc(std::string id, float weight, std::shared_ptr<Place> place, unsigned long priority, float share);
    };
}