#include "Transition.h"

using namespace std;

namespace hpnmg {
    Transition::Transition(std::string id) : id(id) {}

    void Transition::addInputArc(std::shared_ptr<DiscreteArc> &arc) { discreteInputArcs.push_back(arc); }
    void Transition::addInputArc(std::shared_ptr<FluidArc> &arc) { fluidInputArcs.push_back(arc); }
    void Transition::addInputArc(std::shared_ptr<GuardArc> &arc) { guardInputArcs.push_back(arc); }

    void Transition::addOutputArc(std::shared_ptr<DiscreteArc> &arc) { discreteOutputArcs.push_back(arc); }
    void Transition::addOutputArc(std::shared_ptr<FluidArc> &arc) { fluidOutputArcs.push_back(arc); }
    void Transition::addOutputArc(std::shared_ptr<GuardArc> &arc) { guardOutputArcs.push_back(arc); }

    const vector<shared_ptr<DiscreteArc>> Transition::getDiscreteInputArcs() const {
        return discreteInputArcs;
    }

    const vector<shared_ptr<FluidArc>> Transition::getFluidInputArcs() const {
        return fluidInputArcs;
    }

    const vector<shared_ptr<GuardArc>> Transition::getGuardInputArcs() const {
        return guardInputArcs;
    }

    const vector<shared_ptr<DiscreteArc>> Transition::getDiscreteOutputArcs() const {
        return discreteOutputArcs;
    }

    const vector<shared_ptr<FluidArc>> Transition::getFluidOutputArcs() const {
        return fluidOutputArcs;
    }

    const vector<shared_ptr<GuardArc>> Transition::getGuardOutputArcs() const {
        return guardOutputArcs;
    }

//    vector<std::shared_ptr<Arc>> Transition::getInputArcs(){
//        return inputArcs;
//    }
//
//    vector<std::shared_ptr<Arc>> Transition::getOutputArcs() {
//        return outputArcs;
//    }
}