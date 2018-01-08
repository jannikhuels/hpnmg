#include "GeneralTransition.h"

namespace hpnmg {

    bool GeneralTransition::isEnabled() const {
        return enabled;
    }

    void GeneralTransition::setEnabled(bool enabled) {
        GeneralTransition::enabled = enabled;
    }

    double GeneralTransition::getPassedTimeEnabled() const {
        return passedTimeEnabled;
    }

    void GeneralTransition::setPassedTimeEnabled(double passedTimeEnabled) {
        GeneralTransition::passedTimeEnabled = passedTimeEnabled;
    }
}