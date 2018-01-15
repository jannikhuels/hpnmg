#include "DiscreteArc.h"

using namespace std;
namespace hpnmg {

    DiscreteArc::DiscreteArc(string id, float weight, std::shared_ptr<Place> place)
            : Arc(id, weight, place)
    {}
}