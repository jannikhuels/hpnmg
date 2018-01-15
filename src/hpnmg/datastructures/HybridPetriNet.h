#pragma once

#include <vector>
#include <string>
#include <memory>

#include "places/DiscretePlace.h"
#include "places/FluidPlace.h"
#include "places/Place.h"
#include "transitions/DeterministicTransition.h"
#include "transitions/FluidTransition.h"
#include "transitions/GeneralTransition.h"
#include "transitions/ImmediateTransition.h"
#include "transitions/Transition.h"
#include "arcs/Arc.h"
#include "arcs/DiscreteArc.h"
#include "arcs/FluidArc.h"
#include "arcs/GuardArc.h"

using namespace std;
namespace hpnmg {
    class HybridPetriNet {

    private:
        vector<shared_ptr<DiscretePlace>> discretePlaces;
        vector<shared_ptr<FluidPlace>> fluidPlaces;
        vector<shared_ptr<DeterministicTransition>> deterministicTransitions;
        vector<shared_ptr<FluidTransition>> fluidTransitions;
        vector<shared_ptr<GeneralTransition>> generalTransitions;
        vector<shared_ptr<ImmediateTransition>> immediateTransitions;

    public:
        unsigned long num_places();

        unsigned long num_transistions();

        string getNodeTypeByID(string id);

        shared_ptr<Place> getPlaceById(string id);

        shared_ptr<Transition> getTransitionById(string id);

        void addPlace(shared_ptr<DiscretePlace> &place);
        void addPlace(shared_ptr<FluidPlace> &place);

        void addTransition(shared_ptr<DeterministicTransition> &transition);
        void addTransition(shared_ptr<FluidTransition> &transition);
        void addTransition(shared_ptr<GeneralTransition> &transition);
        void addTransition(shared_ptr<ImmediateTransition> &transition);

        void addInputArc(string transitionID, shared_ptr<DiscreteArc> &arc);
        void addInputArc(string transitionID, shared_ptr<FluidArc> &arc);
        void addInputArc(string transitionID, shared_ptr<GuardArc> &arc);

        void addOutputArc(string transitionID, shared_ptr<DiscreteArc> &arc);
        void addOutputArc(string transitionID, shared_ptr<FluidArc> &arc);
        void addOutputArc(string transitionID, shared_ptr<GuardArc> &arc);
    };
}