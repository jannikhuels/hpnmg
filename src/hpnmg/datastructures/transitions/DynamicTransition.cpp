#include "DynamicTransition.h"

namespace hpnmg {
    using namespace std;

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

    double DynamicTransition::getRate() {
        double rate = 0;
        for (int i=0;  i < transitions.size(); i++) {
            rate += transitionFactors[i] * transitions[i]->getRate();
        }
        rate = factor * (rate + constant);

        // maximum function (default right now)
        if (rate >= parameter)  {
            return rate;
        } else {
            return parameter;
        }

    }

    double DynamicTransition::getFactor() {
        return factor;
    }

    double DynamicTransition::getConstant() {
        return constant;
    }

    std::vector<shared_ptr<ContinuousTransition>> DynamicTransition::getTransitions() {
        return transitions;
    }

    std::vector<double> DynamicTransition::getTransitionFactors() {
        return transitionFactors;
    }

    double DynamicTransition::getParameter() {
        return parameter;
    }
}
