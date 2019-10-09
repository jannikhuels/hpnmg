#include "ContinuousNondeterministicTransition.h"

namespace hpnmg {
    using namespace std;

    ContinuousNondeterministicTransition::ContinuousNondeterministicTransition(std::string id, pair<double, double> rateInterval)
        : Transition(id), rateInterval(rateInterval), currentRateInterval(rateInterval)
    {}

    pair<double, double> ContinuousNondeterministicTransition::getRateInterval() { return currentRateInterval; }

    void ContinuousNondeterministicTransition::setCurrentRateInterval(pair<double, double> newRateInterval) { this->currentRateInterval = newRateInterval; };

    void ContinuousNondeterministicTransition::resetOriginalRate() { currentRateInterval = rateInterval; }

}
