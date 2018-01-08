#include "FluidTransition.h"


namespace hpnmg {

    bool FluidTransition::isEnabled() const {
        return enabled;
    }

    void FluidTransition::setEnabled(bool enabled) {
        FluidTransition::enabled = enabled;
    }
}
