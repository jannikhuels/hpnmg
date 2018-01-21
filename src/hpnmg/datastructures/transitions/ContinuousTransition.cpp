#include "ContinuousTransition.h"

using namespace std;
namespace hpnmg {
    ContinuousTransition::ContinuousTransition(string id, double rate)
        : Transition(id), rate(rate)
    {}

    double ContinuousTransition::getRate() { return rate; }
}
