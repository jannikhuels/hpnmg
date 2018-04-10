#pragma once

#include "Arc.h"

namespace hpnmg {
    class ContinuousArc : public Arc {
    private:
        unsigned long priority;
        double share;

    public:
        ContinuousArc(std::string id, float weight, std::shared_ptr<Place> place, unsigned long priority, double share);
        unsigned long getPriority();
        double getShare();
    };
}