#ifndef HPNMG_LINEARRESTRICTION_H
#define HPNMG_LINEARRESTRICTION_H

#include "LinearEquation.h"

namespace hpnmg {
    class LinearRestriction {
    public:
        LinearEquation solutionOne;
        LinearEquation solutionTwo;

        LinearRestriction();
        LinearRestriction(LinEq left, LinEq center, LinEq right);
        LinearRestriction(const LinearRestriction &linearRestriction);

        bool alwaysFalse();
    };
}

#endif //HPNMG_LINEARRESTRICTION_H
