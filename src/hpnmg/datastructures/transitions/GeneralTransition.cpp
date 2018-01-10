#include "GeneralTransition.h"

namespace hpnmg {
    GeneralTransition::GeneralTransition(std::string id, unsigned long priority, float weight)
        : Transition(id), priority(priority), weight(weight)
    {}
}