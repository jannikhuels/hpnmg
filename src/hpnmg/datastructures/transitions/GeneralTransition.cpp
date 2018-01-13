#include "GeneralTransition.h"

using namespace std;
namespace hpnmg {
    GeneralTransition::GeneralTransition(std::string id, unsigned long priority, float weight, string cdf, map<string, float> parameter)
        : Transition(id), priority(priority), weight(weight), cdf(cdf), parameter(parameter)
    {}
}