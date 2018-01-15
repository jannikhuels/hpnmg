#include "FluidArc.h"

using namespace std;
namespace hpnmg {
    FluidArc::FluidArc(string id, float weight, std::shared_ptr<Place> place, unsigned long priority, float share)
            : Arc(id, weight, place), priority(priority), share(share) {}
}