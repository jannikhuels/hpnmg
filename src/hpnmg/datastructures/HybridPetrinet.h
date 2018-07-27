#pragma once

#include <string>
#include <memory>

#include "places/DiscretePlace.h"
#include "places/ContinuousPlace.h"
#include "places/Place.h"
#include "transitions/DeterministicTransition.h"
#include "transitions/ContinuousTransition.h"
#include "transitions/GeneralTransition.h"
#include "transitions/ImmediateTransition.h"
#include "transitions/Transition.h"
#include "arcs/Arc.h"
#include "arcs/DiscreteArc.h"
#include "arcs/ContinuousArc.h"
#include "arcs/GuardArc.h"

using namespace std;
namespace hpnmg {
    class HybridPetrinet {

    private:
        map<string, shared_ptr<DiscretePlace>> discretePlaces;
        map<string, shared_ptr<ContinuousPlace>> continuousPlaces;
        map<string, shared_ptr<DeterministicTransition>> deterministicTransitions;
        map<string, shared_ptr<ContinuousTransition>> continuousTransitions;
        map<string, shared_ptr<GeneralTransition>> generalTransitions;
        map<string, shared_ptr<ImmediateTransition>> immediateTransitions;

    public:
        unsigned long num_places();
        unsigned long num_transitions();

        unsigned long num_discrete_places();
        unsigned long num_continuous_places();
        unsigned long num_general_transitions();

        string getNodeTypeByID(string id);
        shared_ptr<Place> getPlaceById(string id);

        shared_ptr<Transition> getTransitionById(string id);

        map<string, shared_ptr<DiscretePlace>> getDiscretePlaces();
        map<string, shared_ptr<ContinuousPlace>> getContinuousPlaces();
        map<string, shared_ptr<DeterministicTransition>> getDeterministicTransitions();
        map<string, shared_ptr<ContinuousTransition>> getContinuousTransitions();
        map<string, shared_ptr<GeneralTransition>> getGeneralTransitions();
        map<string, shared_ptr<ImmediateTransition>> getImmediateTransitions();

        void addPlace(shared_ptr<DiscretePlace> &place);
        void addPlace(shared_ptr<ContinuousPlace> &place);

        void addTransition(shared_ptr<DeterministicTransition> &transition);
        void addTransition(shared_ptr<ContinuousTransition> &transition);
        void addTransition(shared_ptr<DynamicTransition> &transition);
        void addTransition(shared_ptr<GeneralTransition> &transition);
        void addTransition(shared_ptr<ImmediateTransition> &transition);

        void addInputArc(string transitionID, shared_ptr<DiscreteArc> &arc);
        void addInputArc(string transitionID, shared_ptr<ContinuousArc> &arc);
        void addInputArc(string transitionID, shared_ptr<GuardArc> &arc);

        void addOutputArc(string transitionID, shared_ptr<DiscreteArc> &arc);
        void addOutputArc(string transitionID, shared_ptr<ContinuousArc> &arc);
        void addOutputArc(string transitionID, shared_ptr<GuardArc> &arc);
    };
}