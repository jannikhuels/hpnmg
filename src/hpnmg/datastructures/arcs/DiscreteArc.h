#pragma once

#include "Arc.h"

namespace hpnmg {
    class DiscreteArc : public Arc {

    public:
        DiscreteArc(std::string id, float weight, std::shared_ptr<Place> place);
    };
}