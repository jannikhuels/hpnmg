//
// Created by Jannik Hüls on 09.01.19.
//

#include "LinearEquation.h"

namespace hpnmg {
    LinearEquation::LinearEquation() {
        equation = {};
        dependencyIndex = -1;
        isUpper = true;
        alwaysTrue = false;
    }

    LinearEquation::LinearEquation(LinEq eq, bool upper, int dIndex) {
        equation = eq;
        isUpper = upper;
        dependencyIndex = dIndex;
        alwaysTrue = false;
    }

    LinearEquation::LinearEquation(LinEq left, LinEq right) {
        int depIndex = Computation::getDependencyIndex(left, right);
        if (depIndex < 0) {
            alwaysTrue = left[0] <= right[0];
            dependencyIndex = depIndex;
        } else {
            isUpper = Computation::isUpper(left, right, depIndex);
            equation = Computation::computeUnequationCut(left, right, depIndex);
            dependencyIndex = depIndex - 1;
            alwaysTrue = false;
        }
    }

    LinearEquation::LinearEquation(const hpnmg::LinearEquation &equation) {
        this->dependencyIndex = equation.dependencyIndex;
        this->alwaysTrue = equation.alwaysTrue;
        this->equation = equation.equation;
        this->isUpper = equation.isUpper;
    }

    bool LinearEquation::isDecidable() {
        return dependencyIndex <= 0;
    }
}
