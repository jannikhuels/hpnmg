#include "ContinuousTransition.h"


namespace hpnmg {
    ContinuousTransition::ContinuousTransition(std::string id, float rate)
        : Transition(id), rate(rate)
    {}
}
