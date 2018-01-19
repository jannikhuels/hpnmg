#pragma once

#include "Arc.h"

namespace hpnmg {
    class ContinuousArc : public Arc {
    private:
        unsigned long priority;
        float share;

    public:
        ContinuousArc(std::string id, float weight, std::shared_ptr<Place> place, unsigned long priority, float share);
    };
}