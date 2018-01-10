#include "FluidPlace.h"


namespace hpnmg {
    FluidPlace::FluidPlace(std::string id, float capacity, float level, bool infinitecapacity)
            : Place(id), capacity(capacity), level(level), infiniteCapacity(infinitecapacity)
    {}
}