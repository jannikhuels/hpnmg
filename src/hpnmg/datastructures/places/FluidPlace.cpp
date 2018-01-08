#include "FluidPlace.h"


namespace hpnmg {
    double FluidPlace::getFluidLevel() const {
        return fluidLevel;
    }

    void FluidPlace::setFluidLevel(double fluidLevel) {
        FluidPlace::fluidLevel = fluidLevel;
    }
}