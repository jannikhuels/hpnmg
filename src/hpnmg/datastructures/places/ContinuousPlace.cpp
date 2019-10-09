#include "ContinuousPlace.h"

namespace hpnmg {
    using namespace std;
    ContinuousPlace::ContinuousPlace(string id, double capacity, double level, bool infinitecapacity)
            : Place(id), capacity(capacity), level(level), infiniteCapacity(infinitecapacity)
    {}

    double ContinuousPlace::getLevel() { return level; }
    double ContinuousPlace::getCapacity() { return capacity; }
    bool ContinuousPlace::getInfiniteCapacity() { return infiniteCapacity; }
}