#include "GeneralTransition.h"

using namespace std;
namespace hpnmg {
    GeneralTransition::GeneralTransition(string id, unsigned long priority, float weight, string cdf,
                                         map<string, float> parameter, string policy)
            : Transition(id), priority(priority), weight(weight), cdf(cdf), parameter(parameter), policy(policy) {}

    const string &GeneralTransition::getCdf() const {
        return cdf;
    }

    void GeneralTransition::setCdf(const string &cdf) {
        GeneralTransition::cdf = cdf;
    }

    const map<string, float> &GeneralTransition::getParameter() const {
        return parameter;
    }

    void GeneralTransition::setParameter(const map<string, float> &parameter) {
        GeneralTransition::parameter = parameter;
    }
}