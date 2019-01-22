//
// Created by Jannik Hüls on 09.01.19.
//

#include "LinearRestriction.h"

namespace hpnmg {

    LinearRestriction::LinearRestriction() {
        this->solutionOne = LinearEquation();
        this->solutionTwo = LinearEquation();
    }

    LinearRestriction::LinearRestriction(LinEq left, LinEq center, LinEq right) {
        this->solutionOne = LinearEquation(left, center);
        this->solutionTwo = LinearEquation(center, right);
    }

    LinearRestriction::LinearRestriction(const hpnmg::LinearRestriction &linearRestriction) {
        this->solutionOne = linearRestriction.solutionOne;
        this->solutionTwo = linearRestriction.solutionTwo;
    }
}