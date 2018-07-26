#include "DynamicTransition.h"

using namespace std;
namespace hpnmg {
    DynamicTransition::DynamicTransition(std::string id,
                                         int factor,
                                         double constant,
                                         std::vector<ContinuousTransition> transitions,
                                         std::vector<int> transitionFactors,
                                         int parameter)
        : ContinuousTransition(id, 0) // rate is set to 0 right now
    {}

    double DynamicTransition::getRate() { // why is getRate never used?
        double rate;
        for (int i=0;  i < transitionFactors.size(); i++) {
            rate =+ transitionFactors[i] * transitions[i].getRate();
        }
        rate = factor * (rate + constant);

        if (rate >= parameter)  {
            return rate;
        } else {
            return parameter;
        }

    }
}
