#ifndef HPNMG_LINEAREQUATION_H
#define HPNMG_LINEAREQUATION_H

#include "Computation.h"

namespace hpnmg {
    typedef std::vector<double> LinEq;

    class LinearEquation {
    public:
        int dependencyIndex;
        bool isUpper;
        bool alwaysTrue;
        LinEq equation;

        LinearEquation(LinEq eq, bool upper, int dIndex);

        LinearEquation(LinEq left, LinEq right);

        LinearEquation(const LinearEquation &equation);

        LinearEquation();

        bool isDecidable();



    };
}

#endif //HPNMG_LINEAREQUATION_H
