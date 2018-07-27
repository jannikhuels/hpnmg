#include "DynamicTransition.h"

using namespace std;
namespace hpnmg {
    DynamicTransition::DynamicTransition(std::string id,
                                         double factor,
                                         double constant,
                                         std::vector<shared_ptr<ContinuousTransition>> transitions,
                                         std::vector<double> transitionFactors,
                                         double parameter)
        : ContinuousTransition(id, 0), // rate is set to 0 right now
          factor(factor),
          constant(constant),
          transitions(transitions),
          transitionFactors(transitionFactors),
          parameter(parameter)
    {}

    double DynamicTransition::getRate() { // why is getRate never used?
        double rate;
        for (int i=0;  i < transitionFactors.size(); i++) {
            rate =+ transitionFactors[i] * transitions[i]->getRate();
        }
        rate = factor * (rate + constant);

        // maximum function (default right now)
        if (rate >= parameter)  {
            return rate;
        } else {
            return parameter;
        }

    }
}
