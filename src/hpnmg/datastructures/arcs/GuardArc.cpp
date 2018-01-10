#include "GuardArc.h"

using namespace std;
namespace hpnmg {
    GuardArc::GuardArc(string id, float weight, Transition transition, Place place, bool isInputArc, bool isInhibitor)
            : Arc(id, weight, transition, place, isInputArc), isInhibitor(isInhibitor)
    {}
}