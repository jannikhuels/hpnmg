#include "HybridPetriNet.h"

namespace hpnmg {

    unsigned long HybridPetriNet::num_places() {
        return discretePlaces.size() + fluidPlaces.size();
    }

    unsigned long HybridPetriNet::num_transistions() {
        return deterministicTransitions.size() + fluidTransitions.size() + generalTransitions.size() +
               immediateTransitions.size();
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


}