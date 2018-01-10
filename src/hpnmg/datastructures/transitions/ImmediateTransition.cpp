#include "ImmediateTransition.h"


using namespace std;
namespace hpnmg {
    ImmediateTransition::ImmediateTransition(std::string id, unsigned long priority, float weight)
        : Transition(id), priority(priority), weight(weight)
    {}
}