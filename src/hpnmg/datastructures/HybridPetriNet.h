#pragma once

#include <vector>
#include <string>

#include "places/DiscretePlace.h"
#include "places/FluidPlace.h"
#include "places/Place.h"
#include "transitions/DeterministicTransition.h"
#include "transitions/FluidTransition.h"
#include "transitions/GeneralTransition.h"
#include "transitions/ImmediateTransition.h"
#include "transitions/Transition.h"

namespace hpnmg {
    struct Arc {
        Place place;
        Transition transition;
        std::string arcType;
    };

    class HybridPetriNet {

    private:
        std::vector<DiscretePlace> discretePlaces;
        std::vector<FluidPlace> fluidPlaces;
        std::vector<DeterministicTransition> deterministicTransitions;
        std::vector<FluidTransition> fluidTransitions;
        std::vector<GeneralTransition> generalTransitions;
        std::vector<ImmediateTransition> immediateTransitions;
        std::vector<Arc> inputArcs;
        std::vector<Arc> outputArcs;

    };
}