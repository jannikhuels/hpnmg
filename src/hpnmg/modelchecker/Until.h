#ifndef HPNMG_UNTIL_H
#define HPNMG_UNTIL_H

#include <string>

#include "modelchecker/Formula.h"

namespace hpnmg {
    struct Until {
        Until(Formula pre, Formula goal) : pre(pre), goal(goal) {};

        Formula pre;
        Formula goal;
    };
}


#endif //HPNMG_UNTIL_H
