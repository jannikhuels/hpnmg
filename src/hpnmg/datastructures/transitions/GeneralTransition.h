#pragma once

#include "Transition.h"

namespace hpnmg {
    class GeneralTransition : public Transition {

    private:
        unsigned long priority;
        float weight;
        //todo: add missing attributes

    public:
        GeneralTransition(std::string id, unsigned long priority, float weight);
    };
}