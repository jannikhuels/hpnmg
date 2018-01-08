#include "DeterministicTransition.h"

namespace hpnmg {

    double DeterministicTransition::getPassedTimeEnabled() const {
        return passedTimeEnabled;
    }

    void DeterministicTransition::setPassedTimeEnabled(double passedTimeEnabled) {
        DeterministicTransition::passedTimeEnabled = passedTimeEnabled;
    }

    bool DeterministicTransition::isEnabled() const {
        return enabled;
    }

    void DeterministicTransition::setEnabled(bool enabled) {
        DeterministicTransition::enabled = enabled;
    }
}