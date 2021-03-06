#include "DiscretePlace.h"

namespace hpnmg {
    DiscretePlace::DiscretePlace(std::string id, unsigned long marking)
        : Place(id), marking(marking)
    {}

    unsigned long DiscretePlace::getMarking() { return marking; }
}
