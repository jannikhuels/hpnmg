#include "Arc.h"

namespace hpnmg {
    using namespace std;

    Arc::Arc(string id, float weight, shared_ptr<Place> place)
            : id(id), weight(weight), place(place) {}

}