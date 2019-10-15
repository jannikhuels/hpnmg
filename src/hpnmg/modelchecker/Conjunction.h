#ifndef HPNMG_CONJUNCTION_H
#define HPNMG_CONJUNCTION_H

#include "modelchecker/Formula.h"

#include <string>

namespace hpnmg {
    struct Conjunction {
        Conjunction(Formula left, Formula right) : left(left), right(right) {};

        Formula left;
        Formula right;
    };
}


#endif //HPNMG_CONJUNCTION_H
