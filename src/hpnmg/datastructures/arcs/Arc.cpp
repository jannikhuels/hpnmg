#include "Arc.h"

using namespace std;
namespace hpnmg {

    Arc::Arc(string id, float weight, Transition transition, Place place, bool isInputArc)
            : id(id), weight(weight), transition(transition), place(place), isInputArc(isInputArc) {}

}