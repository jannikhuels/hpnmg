#ifndef HPNMG_UNTIL_H
#define HPNMG_UNTIL_H

#include "modelchecker/Formula.h"

#include <string>
#include <utility>

namespace hpnmg {
    struct Until {
        Until(Formula pre, Formula goal, double withinTime) : pre(std::move(pre)), goal(std::move(goal)), withinTime(std::move(withinTime)) {};

        Formula pre;
        Formula goal;
        double withinTime;
    };
}


#endif //HPNMG_UNTIL_H
