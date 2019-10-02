#pragma once

#include "Transition.h"

namespace hpnmg {
    class ContinuousNondeterministicTransition : public Transition {

    private:
        pair<double,double> rateInterval;

    public:
        pair<double,double> currentRateInterval;
        ContinuousNondeterministicTransition(std::string id, pair<double,double> rateInterval);

        virtual pair<double,double> getRateInterval();
        void setCurrentRateInterval(pair<double,double> newRateInterval);
        void resetOriginalRate();
    };
}