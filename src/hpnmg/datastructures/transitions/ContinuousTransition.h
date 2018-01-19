#pragma once

#include "Transition.h"

namespace hpnmg {
    class ContinuousTransition : public Transition {

    private:
        float rate;

    public:
        ContinuousTransition(std::string id, float rate);
    };
}