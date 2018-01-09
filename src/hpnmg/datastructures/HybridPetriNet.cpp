#include "HybridPetriNet.h"

namespace hpnmg {

    unsigned long HybridPetriNet::num_places() {
        return discretePlaces.size() + fluidPlaces.size();
    }

    void HybridPetriNet::addDiscretePlace(DiscretePlace &discretePlace) {
        discretePlaces.push_back(discretePlace);
    }

    void HybridPetriNet::addFluidPlace(FluidPlace &fluidPlace) {
        fluidPlaces.push_back(fluidPlace);
    }

}