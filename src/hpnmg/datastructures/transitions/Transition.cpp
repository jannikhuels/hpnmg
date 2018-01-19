#include "Transition.h"

using namespace std;
namespace hpnmg {
    Transition::Transition(string &id) : id(id) {}

    void Transition::addInputArc(std::shared_ptr<DiscreteArc> &arc) { discreteInputArcs[arc->id] = arc; }
    void Transition::addInputArc(std::shared_ptr<ContinuousArc> &arc) { continuousInputArcs[arc->id] = arc; }
    void Transition::addInputArc(std::shared_ptr<GuardArc> &arc) { guardInputArcs[arc->id] = arc; }

    void Transition::addOutputArc(std::shared_ptr<DiscreteArc> &arc) { discreteOutputArcs[arc->id] = arc; }
    void Transition::addOutputArc(std::shared_ptr<ContinuousArc> &arc) { continuousOutputArcs[arc->id] = arc; }
    void Transition::addOutputArc(std::shared_ptr<GuardArc> &arc) { guardOutputArcs[arc->id] = arc; }

    const map<string, shared_ptr<DiscreteArc>> Transition::getDiscreteInputArcs() const { return discreteInputArcs; }
    const map<string, shared_ptr<ContinuousArc>> Transition::getContinuousInputArcs() const { return continuousInputArcs; }
    const map<string, shared_ptr<GuardArc>> Transition::getGuardInputArcs() const { return guardInputArcs; }

    const map<string, shared_ptr<DiscreteArc>> Transition::getDiscreteOutputArcs() const { return discreteOutputArcs; }
    const map<string, shared_ptr<ContinuousArc>> Transition::getContinuousOutputArcs() const { return continuousOutputArcs; }
    const map<string, shared_ptr<GuardArc>> Transition::getGuardOutputArcs() const { return guardOutputArcs; }
}