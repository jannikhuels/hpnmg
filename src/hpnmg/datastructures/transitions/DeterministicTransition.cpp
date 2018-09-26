#include "DeterministicTransition.h"

using namespace std;
namespace hpnmg {
    DeterministicTransition::DeterministicTransition(std::string id, unsigned long priority, double weight, double discTime)
        : Transition(id), priority(priority), weight(weight), discTime(discTime)
    {}

    double DeterministicTransition::getDiscTime() { return discTime; }

    double DeterministicTransition::getWeight() { return weight; }

    void DeterministicTransition::setDiscTime(double discTime) {
        this->discTime = discTime;
    }
}