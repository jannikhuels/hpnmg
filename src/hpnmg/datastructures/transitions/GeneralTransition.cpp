#include "GeneralTransition.h"

using namespace std;
namespace hpnmg {
    GeneralTransition::GeneralTransition(string id, unsigned long priority, float weight, string cdf,
                                         map<string, float> parameter, string policy)
            : Transition(id), priority(priority), weight(weight), cdf(cdf), parameter(parameter), policy(policy) {}
}