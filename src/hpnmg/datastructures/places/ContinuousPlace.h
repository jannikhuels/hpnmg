#pragma once

#include "Place.h"

namespace hpnmg {
    using namespace std;
    class ContinuousPlace : public Place {

    private:
        double capacity;
        double level;
        bool infiniteCapacity;

    public:
        ContinuousPlace(string id, double capacity, double level, bool infinitecapacity);
        double getLevel();
        double getCapacity();
        bool getInfiniteCapacity();
    };
}