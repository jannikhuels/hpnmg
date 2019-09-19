#include "ContinuousTransition.h"

using namespace std;
namespace hpnmg {
    ContinuousTransition::ContinuousTransition(string id, double rate)
        : Transition(id), rate(rate), currentRate(rate)
    {}

    double ContinuousTransition::getRate() { return currentRate; }

    void ContinuousTransition::resetOriginalRate() { currentRate = rate; }

    void ContinuousTransition::setCurrentRate(double newRate) { this->currentRate = newRate; };
}
