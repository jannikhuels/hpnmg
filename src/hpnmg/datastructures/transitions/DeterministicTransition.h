#pragma once

#include "Transition.h"

namespace hpnmg {
    class DeterministicTransition : public Transition {

    private:
        double firingTime;
        double passedTimeEnabled;

    public:
        double getPassedTimeEnabled() const;
        void setPassedTimeEnabled(double passedTimeEnabled);
        bool isEnabled() const;
        void setEnabled(bool enabled);
    };
}