#include "ContinuousArc.h"

using namespace std;
namespace hpnmg {
    ContinuousArc::ContinuousArc(string id, float weight, std::shared_ptr<Place> place, unsigned long priority, double share)
            : Arc(id, weight, place), priority(priority), share(share) {}

    unsigned long ContinuousArc::getPriority() { return priority; }
    double ContinuousArc::getShare() { return share; }
}