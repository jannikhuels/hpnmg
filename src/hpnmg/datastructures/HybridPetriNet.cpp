#include "HybridPetriNet.h"

using namespace std;
namespace hpnmg {

    unsigned long HybridPetriNet::num_places() {
        return discretePlaces.size() + fluidPlaces.size();
    }

    unsigned long HybridPetriNet::num_transitions() {
        return deterministicTransitions.size() + fluidTransitions.size() + generalTransitions.size() +
               immediateTransitions.size();
    }

    string HybridPetriNet::getNodeTypeByID(string id) {
        if (discretePlaces.find(id) != discretePlaces.end())
            return "place";
        else if (fluidPlaces.find(id) != fluidPlaces.end())
            return "place";
        else if (deterministicTransitions.find(id) != deterministicTransitions.end())
            return "transition";
        else if(fluidTransitions.find(id) != fluidTransitions.end())
            return "transition";
        else if (generalTransitions.find(id) != generalTransitions.end())
            return "transition";
        else if (immediateTransitions.find(id) != immediateTransitions.end())
            return "transition";
        else
            return "unknown";
    }

    shared_ptr<Place> HybridPetriNet::getPlaceById(string id) {
        if (discretePlaces.find(id) != discretePlaces.end())
            return discretePlaces[id];
        else if (fluidPlaces.find(id) != fluidPlaces.end())
            return fluidPlaces[id];
        else
            return nullptr;
    }

    shared_ptr<Transition> HybridPetriNet::getTransitionById(string id) {
        if (deterministicTransitions.find(id) != deterministicTransitions.end())
            return deterministicTransitions[id];
        else if (fluidTransitions.find(id) != fluidTransitions.end())
            return fluidTransitions[id];
        else if (generalTransitions.find(id) != generalTransitions.end())
            return generalTransitions[id];
        else if (immediateTransitions.find(id) != immediateTransitions.end())
            return immediateTransitions[id];
        else
            return nullptr;
    }

    void HybridPetriNet::addPlace(shared_ptr<DiscretePlace> &place) { discretePlaces[place->id] = place; }
    void HybridPetriNet::addPlace(shared_ptr<FluidPlace> &place) { fluidPlaces[place->id] = place; }

    void HybridPetriNet::addTransition(shared_ptr<DeterministicTransition> &transition) {
        deterministicTransitions[transition->id] = transition;
    }
    void HybridPetriNet::addTransition(shared_ptr<FluidTransition> &transition) {
        fluidTransitions[transition->id] = transition;
    }
    void HybridPetriNet::addTransition(shared_ptr<GeneralTransition> &transition) {
        generalTransitions[transition->id] = transition;
    }
    void HybridPetriNet::addTransition(shared_ptr<ImmediateTransition> &transition) {
        immediateTransitions[transition->id] = transition;
    }

    void HybridPetriNet::addInputArc(std::string transitionID, std::shared_ptr<DiscreteArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addInputArc(arc);
        else if (fluidTransitions.find(transitionID) != fluidTransitions.end())
            fluidTransitions[transitionID]->addInputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addInputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addInputArc(arc);
    }
    void HybridPetriNet::addInputArc(std::string transitionID, std::shared_ptr<FluidArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addInputArc(arc);
        else if (fluidTransitions.find(transitionID) != fluidTransitions.end())
            fluidTransitions[transitionID]->addInputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addInputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addInputArc(arc);
    }
    void HybridPetriNet::addInputArc(std::string transitionID, std::shared_ptr<GuardArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addInputArc(arc);
        else if (fluidTransitions.find(transitionID) != fluidTransitions.end())
            fluidTransitions[transitionID]->addInputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addInputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addInputArc(arc);
    }

    void HybridPetriNet::addOutputArc(std::string transitionID, std::shared_ptr<DiscreteArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addOutputArc(arc);
        else if (fluidTransitions.find(transitionID) != fluidTransitions.end())
            fluidTransitions[transitionID]->addOutputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addOutputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addOutputArc(arc);
    }
    void HybridPetriNet::addOutputArc(std::string transitionID, std::shared_ptr<FluidArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addOutputArc(arc);
        else if (fluidTransitions.find(transitionID) != fluidTransitions.end())
            fluidTransitions[transitionID]->addOutputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addOutputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addOutputArc(arc);
    }
    void HybridPetriNet::addOutputArc(std::string transitionID, std::shared_ptr<GuardArc> &arc) {
        if (deterministicTransitions.find(transitionID) != deterministicTransitions.end())
            deterministicTransitions[transitionID]->addOutputArc(arc);
        else if (fluidTransitions.find(transitionID) != fluidTransitions.end())
            fluidTransitions[transitionID]->addOutputArc(arc);
        else if (generalTransitions.find(transitionID) != generalTransitions.end())
            generalTransitions[transitionID]->addOutputArc(arc);
        else if (immediateTransitions.find(transitionID) != immediateTransitions.end())
            immediateTransitions[transitionID]->addOutputArc(arc);
    }
}