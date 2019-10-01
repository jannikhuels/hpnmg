#ifndef HPNMG_NEGATION_H
#define HPNMG_NEGATION_H

#include <string>

#include "modelchecker/Formula.h"

namespace hpnmg {
    struct Negation {
        Negation(Formula formula) : formula(formula) {};

        Formula formula;
    };
}


#endif //HPNMG_NEGATION_H
