#include "HybridPetrinet.h"

using namespace std;
namespace hpnmg {

    unsigned long HybridPetrinet::num_places() {
        return discretePlaces.size() + continuousPlaces.size();
    }

    unsigned long HybridPetrinet::num_transitions() {
        return deterministicTransitions.size() + continuousTransitions.size() + generalTransitions.size() +
               immediateTransitions.size();
    }

    unsigned long HybridPetrinet::num_discrete_places() {
        return discretePlaces.size();
    }

    unsigned long HybridPetrinet::num_continuous_places() {
        return continuousPlaces.size();
    }

    unsigned long HybridPetrinet::num_general_transitions() {
        return generalTransitions.size();
    }

    map<string, shared_ptr<DiscretePlace>> HybridPetrinet::getDiscretePlaces() { return discretePlaces; };
    map<string, shared_ptr<ContinuousPlace>> HybridPetrinet::getContinuousPlaces() { return continuousPlaces; };
    map<string, shared_ptr<DeterministicTransition>> HybridPetrinet::getDeterministicTransitions() { return deterministicTransitions; };
    map<string, shared_ptr<ContinuousTransition>> HybridPetrinet::getContinuousTransitions() { return continuousTransitions; };
    map<string, shared_ptr<GeneralTransition>> HybridPetrinet::getGeneralTransitions() { return generalTransitions; };
    map<string, shared_ptr<ImmediateTransition>> HybridPetrinet::getImmediateTransitions() { return immediateTransitions; };

    string HybridPetrinet::getNodeTypeByID(string id) {
        if (discretePlaces.find(id) != discretePlaces.end())
            return "place";
        else if (continuousPlaces.find(id) != continuousPlaces.end())
            return "place";
        else if (deterministicTransitions.find(id) != deterministicTransitions.end())
            return "transition";
        else if(continuousTransitions.find(id) != continuousTransitions.end())
            return "transition";
        else if (generalTransitions.find(id) != generalTransitions.end())
            return "transition";
        else if (immediateTransitions.find(id) != immediateTransitions.end())
            return "transition";
        else
            return "unknown";
    }

    shared_ptr<Place> HybridPetrinet::getPlaceById(string id) {
        if (discretePlaces.find(id) != discretePlaces.end())
            return discretePlaces[id];
        else if (continuousPlaces.find(id) != continuousPlaces.end())
            return continuousPlaces[id];
        else
            return nullptr;
    }

    shared_ptr<Transition> HybridPetrinet::getTransitionById(string id) {
        if (deterministicTransitions.find(id) != deterministicTransitions.end())
            return deterministicTransitions[id];
        else if (continuousTransitions.find(id) != continuousTransitions.end())
            return continuousTransitions[id];
        else if (generalTransitions.find(id) != generalTransitions.end())
            return generalTransitions[id];
        else if (immediateTransitions.find(id) != immediateTransitions.end())
            return immediateTransitions[id];
        else
            return nullptr;
    }

    void HybridPetrinet::addPlace(shared_ptr<DiscretePlace> &place) { discretePlaces[place->id] = place; }
    void HybridPetrinet::addPlace(shared_ptr<ContinuousPlace> &place) { continuousPlaces[place->id] = place; }

    void HybridPetrinet::addTransition(shared_ptr<DeterministicTransition> &transition) {
        deterministicTransitions[transition->id] = transition;
    }
    void HybridPetrinet::addTransition(shared_ptr<ContinuousTransition> &transition) {
        continuousTransitions[transition->id] = transition;
    }
    void HybridPetrinet::addTransition(shared_ptr<GeneralTransition> &transition) {
        generalTransitions[transition->id] = transition;
    }
    void HybridPetrinet::addTransition(shared_ptr<ImmediateTransition> &transition) {
        immediateTransitions[transition->id] = transition;
    }

    void HybridPetrinet::addInputArc(std::string transitionID, std::shared_ptr<DiscreteArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addInputArc(arc);
        else if (continuousTransitions.find(transitionID) != continuousTransitions.end())
            continuousTransitions[transitionID]->addInputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addInputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addInputArc(arc);
    }
    void HybridPetrinet::addInputArc(std::string transitionID, std::shared_ptr<ContinuousArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addInputArc(arc);
        else if (continuousTransitions.find(transitionID) != continuousTransitions.end())
            continuousTransitions[transitionID]->addInputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addInputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addInputArc(arc);
    }
    void HybridPetrinet::addInputArc(std::string transitionID, std::shared_ptr<GuardArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addInputArc(arc);
        else if (continuousTransitions.find(transitionID) != continuousTransitions.end())
            continuousTransitions[transitionID]->addInputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addInputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addInputArc(arc);
    }

    void HybridPetrinet::addOutputArc(std::string transitionID, std::shared_ptr<DiscreteArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addOutputArc(arc);
        else if (continuousTransitions.find(transitionID) != continuousTransitions.end())
            continuousTransitions[transitionID]->addOutputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addOutputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addOutputArc(arc);
    }
    void HybridPetrinet::addOutputArc(std::string transitionID, std::shared_ptr<ContinuousArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addOutputArc(arc);
        else if (continuousTransitions.find(transitionID) != continuousTransitions.end())
            continuousTransitions[transitionID]->addOutputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addOutputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addOutputArc(arc);
    }
    void HybridPetrinet::addOutputArc(std::string transitionID, std::shared_ptr<GuardArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addOutputArc(arc);
        else if (continuousTransitions.find(transitionID) != continuousTransitions.end())
            continuousTransitions[transitionID]->addOutputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addOutputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addOutputArc(arc);
    }
}