#pragma once

#include "Arc.h"

namespace hpnmg {
    class GuardArc : public Arc {
    private:
        bool isInhibitor;

    public:
        GuardArc(std::string id, float weight, Transition transition, Place place, bool isInputArc, bool isInhibitor);
    };
}