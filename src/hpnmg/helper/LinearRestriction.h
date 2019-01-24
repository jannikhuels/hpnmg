//
// Created by Jannik Hüls on 09.01.19.
//

#ifndef HPNMG_LINEARRESTRICTION_H
#define HPNMG_LINEARRESTRICTION_H

#include "LinearEquation.h"
#include "Computation.h"

namespace hpnmg {
    class LinearRestriction {
    public:
        LinearEquation solutionOne;
        LinearEquation solutionTwo;

        LinearRestriction();
        LinearRestriction(LinEq left, LinEq center, LinEq right);
        LinearRestriction(const LinearRestriction &linearRestriction);
    };
}

#endif //HPNMG_LINEARRESTRICTION_H
