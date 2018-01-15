#include "GuardArc.h"

using namespace std;
namespace hpnmg {
    GuardArc::GuardArc(string id, float weight, std::shared_ptr<Place> place, bool isInhibitor)
            : Arc(id, weight, place), isInhibitor(isInhibitor)
    {}
}