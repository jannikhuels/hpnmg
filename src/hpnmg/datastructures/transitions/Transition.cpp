#include "Transition.h"

using namespace std;
namespace hpnmg {
    Transition::Transition(string &id) : id(id) {}

    void Transition::addInputArc(std::shared_ptr<DiscreteArc> &arc) { discreteInputArcs[arc->id] = arc; }
    void Transition::addInputArc(std::shared_ptr<FluidArc> &arc) { fluidInputArcs[arc->id] = arc; }
    void Transition::addInputArc(std::shared_ptr<GuardArc> &arc) { guardInputArcs[arc->id] = arc; }

    void Transition::addOutputArc(std::shared_ptr<DiscreteArc> &arc) { discreteOutputArcs[arc->id] = arc; }
    void Transition::addOutputArc(std::shared_ptr<FluidArc> &arc) { fluidOutputArcs[arc->id] = arc; }
    void Transition::addOutputArc(std::shared_ptr<GuardArc> &arc) { guardOutputArcs[arc->id] = arc; }

    const map<string, shared_ptr<DiscreteArc>> Transition::getDiscreteInputArcs() const { return discreteInputArcs; }
    const map<string, shared_ptr<FluidArc>> Transition::getFluidInputArcs() const { return fluidInputArcs; }
    const map<string, shared_ptr<GuardArc>> Transition::getGuardInputArcs() const { return guardInputArcs; }

    const map<string, shared_ptr<DiscreteArc>> Transition::getDiscreteOutputArcs() const { return discreteOutputArcs; }
    const map<string, shared_ptr<FluidArc>> Transition::getFluidOutputArcs() const { return fluidOutputArcs; }
    const map<string, shared_ptr<GuardArc>> Transition::getGuardOutputArcs() const { return guardOutputArcs; }
}