#include "FluidArc.h"

using namespace std;
namespace hpnmg {
    FluidArc::FluidArc(string id, float weight, Transition transition, Place place, bool isInputArc,
                       unsigned long priority, float share)
            : Arc(id, weight, transition, place, isInputArc), priority(priority), share(share) {}
}