#pragma once

#include "Transition.h"

namespace hpnmg {
    class ImmediateTransition : public Transition {

    public:
        bool isEnabled() const;
        void setEnabled(bool enabled);
    };
}
