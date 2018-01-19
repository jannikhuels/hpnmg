#include "ContinuousArc.h"

using namespace std;
namespace hpnmg {
    ContinuousArc::ContinuousArc(string id, float weight, std::shared_ptr<Place> place, unsigned long priority, float share)
            : Arc(id, weight, place), priority(priority), share(share) {}
}