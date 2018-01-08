#pragma once

#include "Transition.h"

namespace hpnmg {
    class GeneralTransition : public Transition {

    private:
        double passedTimeEnabled;

    public:
        double getPassedTimeEnabled() const;
        void setPassedTimeEnabled(double passedTimeEnabled);
        bool isEnabled() const;
        void setEnabled(bool enabled);
    };
}