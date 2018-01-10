#include "DiscreteArc.h"

using namespace std;
namespace hpnmg {

    DiscreteArc::DiscreteArc(string id, float weight, Transition transition, Place place, bool isInputArc)
            : Arc(id, weight, transition, place, isInputArc)
    {}
}