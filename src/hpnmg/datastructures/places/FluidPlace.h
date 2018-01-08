#pragma once

#include "Place.h"


namespace hpnmg {
    class FluidPlace : public Place {

    private:
        double capacity;
        double fluidLevel;
        double initialFluidLevel;

    public:
        double getFluidLevel() const;

        void setFluidLevel(double fluidLevel);
    };
}