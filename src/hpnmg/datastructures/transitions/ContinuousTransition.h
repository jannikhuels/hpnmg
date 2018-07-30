#pragma once

#include "Transition.h"

namespace hpnmg {
    class ContinuousTransition : public Transition {

    private:
        double rate;
        double currentRate;

    public:
        ContinuousTransition(std::string id, double rate);
        double getRate();
        void setCurrentRate(double newRate);
    };
}