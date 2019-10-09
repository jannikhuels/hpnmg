#include "DeterministicTransition.h"

namespace hpnmg {
    using namespace std;

    DeterministicTransition::DeterministicTransition(std::string id, unsigned long priority, double weight, double discTime)
        : Transition(id), priority(priority), weight(weight), discTime(discTime)
    {}

    double DeterministicTransition::getDiscTime() { return discTime; }

    double DeterministicTransition::getWeight() { return weight; }

    double DeterministicTransition::getPriority() { return priority; }

    void DeterministicTransition::setDiscTime(double discTime) {
        this->discTime = discTime;
    }
}