#include "HybridPetriNet.h"

using namespace std;
namespace hpnmg {

    unsigned long HybridPetriNet::num_places() {
        return discretePlaces.size() + fluidPlaces.size();
    }

    unsigned long HybridPetriNet::num_transistions() {
        return deterministicTransitions.size() + fluidTransitions.size() + generalTransitions.size() +
               immediateTransitions.size();
    }

    string HybridPetriNet::getNodeTypeByID(string id) {
        for(shared_ptr<DiscretePlace> &place : discretePlaces)
            if (place->id == id)
                return "place";
        for(shared_ptr<FluidPlace> &place : fluidPlaces)
            if (place->id == id)
                return "place";
        for(shared_ptr<DeterministicTransition> &transition : deterministicTransitions)
            if (transition->id == id)
                return "transition";
        for(shared_ptr<FluidTransition> &transition : fluidTransitions)
            if (transition->id == id)
                return "transition";
        for(shared_ptr<GeneralTransition> &transition : generalTransitions)
            if (transition->id == id)
                return "transition";
        for(shared_ptr<ImmediateTransition> &transition : immediateTransitions)
            if (transition->id == id)
                return "transition";
        return "unknown";
    }

    shared_ptr<Place> HybridPetriNet::getPlaceById(std::string id) {
        for(auto place : discretePlaces)
            if (place->id == id)
                return place;
        for(auto place : fluidPlaces)
            if (place->id == id)
                return place;
    }

    shared_ptr<Transition> HybridPetriNet::getTransitionById(std::string id) {
        for(auto transition : deterministicTransitions)
            if (transition->id == id)
                return transition;
        for(auto transition : fluidTransitions)
            if (transition->id == id)
                return transition;
        for(auto transition : generalTransitions)
            if (transition->id == id)
                return transition;
        for(auto transition : immediateTransitions)
            if (transition->id == id)
                return transition;
    }

    void HybridPetriNet::addPlace(shared_ptr<DiscretePlace> &place) { discretePlaces.push_back(place); }
    void HybridPetriNet::addPlace(shared_ptr<FluidPlace> &place) { fluidPlaces.push_back(place); }

    void HybridPetriNet::addTransition(shared_ptr<DeterministicTransition> &transition) {
        deterministicTransitions.push_back(transition);
    }
    void HybridPetriNet::addTransition(shared_ptr<FluidTransition> &transition) {
        fluidTransitions.push_back(transition);
    }
    void HybridPetriNet::addTransition(shared_ptr<GeneralTransition> &transition) {
        generalTransitions.push_back(transition);
    }
    void HybridPetriNet::addTransition(shared_ptr<ImmediateTransition> &transition) {
        immediateTransitions.push_back(transition);
    }

    void HybridPetriNet::addInputArc(std::string transitionID, std::shared_ptr<DiscreteArc> &arc) {
        for(shared_ptr<DeterministicTransition> &transition : deterministicTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
        for(shared_ptr<FluidTransition> &transition : fluidTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
        for(shared_ptr<GeneralTransition> &transition : generalTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
        for(shared_ptr<ImmediateTransition> &transition : immediateTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
    }
    void HybridPetriNet::addInputArc(std::string transitionID, std::shared_ptr<FluidArc> &arc) {
        for(shared_ptr<DeterministicTransition> &transition : deterministicTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
        for(shared_ptr<FluidTransition> &transition : fluidTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
        for(shared_ptr<GeneralTransition> &transition : generalTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
        for(shared_ptr<ImmediateTransition> &transition : immediateTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
    }
    void HybridPetriNet::addInputArc(std::string transitionID, std::shared_ptr<GuardArc> &arc) {
        for(shared_ptr<DeterministicTransition> &transition : deterministicTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
        for(shared_ptr<FluidTransition> &transition : fluidTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
        for(shared_ptr<GeneralTransition> &transition : generalTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
        for(shared_ptr<ImmediateTransition> &transition : immediateTransitions)
            if (transition->id == transitionID)
                transition->addInputArc(arc);
    }

    void HybridPetriNet::addOutputArc(std::string transitionID, std::shared_ptr<DiscreteArc> &arc) {
        for(shared_ptr<DeterministicTransition> &transition : deterministicTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
        for(shared_ptr<FluidTransition> &transition : fluidTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
        for(shared_ptr<GeneralTransition> &transition : generalTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
        for(shared_ptr<ImmediateTransition> &transition : immediateTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
    }
    void HybridPetriNet::addOutputArc(std::string transitionID, std::shared_ptr<FluidArc> &arc) {
        for(shared_ptr<DeterministicTransition> &transition : deterministicTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
        for(shared_ptr<FluidTransition> &transition : fluidTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
        for(shared_ptr<GeneralTransition> &transition : generalTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
        for(shared_ptr<ImmediateTransition> &transition : immediateTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
    }
    void HybridPetriNet::addOutputArc(std::string transitionID, std::shared_ptr<GuardArc> &arc) {
        for(shared_ptr<DeterministicTransition> &transition : deterministicTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
        for(shared_ptr<FluidTransition> &transition : fluidTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
        for(shared_ptr<GeneralTransition> &transition : generalTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
        for(shared_ptr<ImmediateTransition> &transition : immediateTransitions)
            if (transition->id == transitionID)
                transition->addOutputArc(arc);
    }
}