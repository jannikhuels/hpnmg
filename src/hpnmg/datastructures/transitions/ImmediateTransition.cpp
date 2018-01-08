#include "ImmediateTransition.h"


namespace hpnmg {

    bool ImmediateTransition::isEnabled() const {
        return enabled;
    }

    void ImmediateTransition::setEnabled(bool enabled) {
        ImmediateTransition::enabled = enabled;
    }
}