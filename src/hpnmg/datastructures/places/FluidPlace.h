#pragma once

#include "Place.h"


namespace hpnmg {
    class FluidPlace : public Place {

    private:
        float capacity;
        float level;
        bool infiniteCapacity;

    public:
        FluidPlace(std::string id, float capacity, float level, bool infinitecapacity);
    };
}