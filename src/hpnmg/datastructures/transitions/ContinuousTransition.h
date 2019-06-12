#pragma once

#include "Transition.h"

namespace hpnmg {
    class ContinuousTransition : public Transition {

    private:
        double rate;

    public:
        double currentRate;
        ContinuousTransition(std::string id, double rate);

        virtual double getRate();

        double getCurrentRate();
        void setCurrentRate(double newRate);
    };
}