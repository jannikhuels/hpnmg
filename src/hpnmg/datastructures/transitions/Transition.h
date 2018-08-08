#pragma once

#include <string>
#include <memory>
#include <map>
#include "../arcs/DiscreteArc.h"
#include "../arcs/ContinuousArc.h"
#include "../arcs/GuardArc.h"

using namespace std;
namespace hpnmg {
    class Transition {
    private:
        map<string, shared_ptr<DiscreteArc>> discreteInputArcs;
        map<string, shared_ptr<ContinuousArc>> continuousInputArcs;
        map<string, shared_ptr<GuardArc>> guardInputArcs;
        map<string, shared_ptr<DiscreteArc>> discreteOutputArcs;
        map<string, shared_ptr<ContinuousArc>> continuousOutputArcs;
        map<string, shared_ptr<GuardArc>> guardOutputArcs;

    public:
        string id;
        virtual string getId(); // virtual method for dynamic cast... (todo)

        Transition(string &id);

        void addInputArc(shared_ptr<DiscreteArc> &arc);
        void addInputArc(shared_ptr<ContinuousArc> &arc);
        void addInputArc(shared_ptr<GuardArc> &arc);

        void addOutputArc(shared_ptr<DiscreteArc> &arc);
        void addOutputArc(shared_ptr<ContinuousArc> &arc);
        void addOutputArc(shared_ptr<GuardArc> &arc);

        const map<string, shared_ptr<DiscreteArc>> getDiscreteInputArcs() const;
        const map<string, shared_ptr<ContinuousArc>> getContinuousInputArcs() const;
        const map<string, shared_ptr<GuardArc>> getGuardInputArcs() const;

        const map<string, shared_ptr<DiscreteArc>> getDiscreteOutputArcs() const;
        const map<string, shared_ptr<ContinuousArc>> getContinuousOutputArcs() const;
        const map<string, shared_ptr<GuardArc>> getGuardOutputArcs() const;
    };
}
