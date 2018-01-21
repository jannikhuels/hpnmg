#pragma once

#include "Place.h"

using namespace std;
namespace hpnmg {
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