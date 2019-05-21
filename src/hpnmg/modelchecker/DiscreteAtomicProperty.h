#include <utility>

#ifndef HPNMG_DISCRETEATOMICPROPERTY_H
#define HPNMG_DISCRETEATOMICPROPERTY_H

#include <string>

namespace hpnmg {
    struct DiscreteAtomicProperty {
        DiscreteAtomicProperty(std::string place, int value) : place(std::move(place)), value(value) {};

        std::string place;
        int value;
    };
}


#endif //HPNMG_DISCRETEATOMICPROPERTY_H
