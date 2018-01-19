#include "ContinuousPlace.h"

using namespace std;
namespace hpnmg {
    ContinuousPlace::ContinuousPlace(string id, double capacity, double level, bool infinitecapacity)
            : Place(id), capacity(capacity), level(level), infiniteCapacity(infinitecapacity)
    {}

    double ContinuousPlace::getLevel() { return level; }
}