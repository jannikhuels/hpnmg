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

    unsigned long HybridPetriNet::num_arcs() {
        return discreteArcs.size() + fluidArcs.size() + guardArcs.size();
    }

    string HybridPetriNet::getNodeTypeByID(string id) {
        for(auto place : discretePlaces)
            if (place.id == id)
                return "place";
        for(auto place : fluidPlaces)
            if (place.id == id)
                return "place";
        for(auto transition : deterministicTransitions)
            if (transition.id == id)
                return "transition";
        for(auto transition : fluidTransitions)
            if (transition.id == id)
                return "transition";
        for(auto transition : generalTransitions)
            if (transition.id == id)
                return "transition";
        for(auto transition : immediateTransitions)
            if (transition.id == id)
                return "transition";
        return "unknown";
    }


    Place HybridPetriNet::getPlaceById(std::string id) {
        for(auto place : discretePlaces)
            if (place.id == id)
                return place;
        for(auto place : fluidPlaces)
            if (place.id == id)
                return place;
    }

    Transition HybridPetriNet::getTransitionById(std::string id) {
        for(auto transition : deterministicTransitions)
            if (transition.id == id)
                return transition;
        for(auto transition : fluidTransitions)
            if (transition.id == id)
                return transition;
        for(auto transition : generalTransitions)
            if (transition.id == id)
                return transition;
        for(auto transition : immediateTransitions)
            if (transition.id == id)
                return transition;
    }

    void HybridPetriNet::addDiscretePlace(DiscretePlace &place) {
        discretePlaces.push_back(place);
    }

    void HybridPetriNet::addFluidPlace(FluidPlace &place) {
        fluidPlaces.push_back(place);
    }

    void HybridPetriNet::addDeterministicTransition(DeterministicTransition &transition) {
        deterministicTransitions.push_back(transition);
    }

    void HybridPetriNet::addFluidTransition(FluidTransition &transition) {
        fluidTransitions.push_back(transition);
    }

    void HybridPetriNet::addGeneralTransition(GeneralTransition &transition) {
        generalTransitions.push_back(transition);
    }

    void HybridPetriNet::addImmediateTransition(ImmediateTransition &transition) {
        immediateTransitions.push_back(transition);
    }

    void HybridPetriNet::addDiscreteArc(DiscreteArc &arc) {
        discreteArcs.push_back(arc);
    }

    void HybridPetriNet::addFluidArc(FluidArc &arc) {
        fluidArcs.push_back(arc);
    }

    void HybridPetriNet::addGuardArc(GuardArc &arc) {
        guardArcs.push_back(arc);
    }
}