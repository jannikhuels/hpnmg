#include "GuardArc.h"

namespace hpnmg {
    using namespace std;
    GuardArc::GuardArc(string id, float weight, std::shared_ptr<Place> place, bool isInhibitor)
            : Arc(id, weight, place), isInhibitor(isInhibitor)
    {}

    bool GuardArc::getIsInhibitor() { return isInhibitor; }
}