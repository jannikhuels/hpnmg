#pragma once

#include "Arc.h"

namespace hpnmg {
    class DiscreteArc : public Arc {

    public:
        DiscreteArc(std::string id, float weight, Transition transition, Place place, bool isInputArc);
    };
}