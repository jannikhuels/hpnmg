#pragma once

#include "Transition.h"

namespace hpnmg {
    class FluidTransition : public Transition {

    private:
        double flowRate;

    public:
        bool isEnabled() const;
        void setEnabled(bool enabled);
    };
}