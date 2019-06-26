#ifndef HPNMG_CONTINUOUSATOMICPROPERTY_H
#define HPNMG_CONTINUOUSATOMICPROPERTY_H

#include <string>
#include <utility>

namespace hpnmg {
    struct ContinuousAtomicProperty {
        ContinuousAtomicProperty(std::string place, int value) : place(std::move(place)), value(value) {};

        std::string place;
        int value;
    };
}


#endif //HPNMG_CONTINUOUSATOMICPROPERTY_H
