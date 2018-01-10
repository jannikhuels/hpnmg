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
#include "arcs/Arc.h"
#include "arcs/DiscreteArc.h"
#include "arcs/FluidArc.h"
#include "arcs/GuardArc.h"

namespace hpnmg {
    class HybridPetriNet {

    public:
        unsigned long num_places();

        unsigned long num_transistions();

        unsigned long num_arcs();

        std::string getNodeTypeByID(std::string id);

        Place getPlaceById(std::string id);

        Transition getTransitionById(std::string id);

        void addDiscretePlace(DiscretePlace &place);

        void addFluidPlace(FluidPlace &place);

        void addDeterministicTransition(DeterministicTransition &transition);

        void addFluidTransition(FluidTransition &transition);

        void addGeneralTransition(GeneralTransition &transition);

        void addImmediateTransition(ImmediateTransition &transition);

        void addDiscreteArc(DiscreteArc &arc);

        void addFluidArc(FluidArc &arc);

        void addGuardArc(GuardArc &arc);

    private:
        std::vector<DiscretePlace> discretePlaces;
        std::vector<FluidPlace> fluidPlaces;
        std::vector<DeterministicTransition> deterministicTransitions;
        std::vector<FluidTransition> fluidTransitions;
        std::vector<GeneralTransition> generalTransitions;
        std::vector<ImmediateTransition> immediateTransitions;
        std::vector<DiscreteArc> discreteArcs;
        std::vector<FluidArc> fluidArcs;
        std::vector<GuardArc> guardArcs;

    };
}