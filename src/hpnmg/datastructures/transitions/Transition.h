#pragma once

#include <string>
#include <vector>
#include <memory>
#include "../arcs/DiscreteArc.h"
#include "../arcs/FluidArc.h"
#include "../arcs/GuardArc.h"

namespace hpnmg {
    class Transition {
    private:
        std::vector<std::shared_ptr<DiscreteArc>> discreteInputArcs;
        std::vector<std::shared_ptr<FluidArc>> fluidInputArcs;
        std::vector<std::shared_ptr<GuardArc>> guardInputArcs;
        std::vector<std::shared_ptr<DiscreteArc>> discreteOutputArcs;
        std::vector<std::shared_ptr<FluidArc>> fluidOutputArcs;
        std::vector<std::shared_ptr<GuardArc>> guardOutputArcs;

    public:
        std::string id;

        Transition(std::string id);

        void addInputArc(std::shared_ptr<DiscreteArc> &arc);
        void addInputArc(std::shared_ptr<FluidArc> &arc);
        void addInputArc(std::shared_ptr<GuardArc> &arc);

        void addOutputArc(std::shared_ptr<DiscreteArc> &arc);
        void addOutputArc(std::shared_ptr<FluidArc> &arc);
        void addOutputArc(std::shared_ptr<GuardArc> &arc);

        const std::vector<std::shared_ptr<DiscreteArc>> getDiscreteInputArcs() const;

        const std::vector<std::shared_ptr<FluidArc>> getFluidInputArcs() const;

        const std::vector<std::shared_ptr<GuardArc>> getGuardInputArcs() const;

        const std::vector<std::shared_ptr<DiscreteArc>> getDiscreteOutputArcs() const;

        const std::vector<std::shared_ptr<FluidArc>> getFluidOutputArcs() const;

        const std::vector<std::shared_ptr<GuardArc>> getGuardOutputArcs() const;
    };
}
