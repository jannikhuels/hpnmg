#include "DeterministicTransition.h"

using namespace std;
namespace hpnmg {
    DeterministicTransition::DeterministicTransition(std::string id, unsigned long priority, float weight, float discTime)
        : Transition(id), priority(priority), weight(weight), discTime(discTime)
    {}
}