#pragma once

#include "Arc.h"

namespace hpnmg {
    class GuardArc : public Arc {
    private:
        bool isInhibitor;

    public:
        GuardArc(std::string id, float weight, std::shared_ptr<Place> place, bool isInhibitor);
        bool getIsInhibitor();
    };
}