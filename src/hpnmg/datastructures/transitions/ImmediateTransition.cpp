#include "ImmediateTransition.h"


using namespace std;
namespace hpnmg {
    ImmediateTransition::ImmediateTransition(std::string id, unsigned long priority, float weight)
        : Transition(id), priority(priority), weight(weight)
    {}

    unsigned long ImmediateTransition::getPriority() { return priority; }
    float ImmediateTransition::getWeight() { return weight; }
}