#include "DiscreteArc.h"

namespace hpnmg {
    using namespace std;

    DiscreteArc::DiscreteArc(string id, float weight, std::shared_ptr<Place> place)
            : Arc(id, weight, place)
    {}
}