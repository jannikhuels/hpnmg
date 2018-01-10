#include "FluidTransition.h"


namespace hpnmg {
    FluidTransition::FluidTransition(std::string id, float rate)
        : Transition(id), rate(rate)
    {}
}
