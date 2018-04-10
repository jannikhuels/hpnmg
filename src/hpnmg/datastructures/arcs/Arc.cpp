#include "Arc.h"

using namespace std;
namespace hpnmg {

    Arc::Arc(string id, float weight, shared_ptr<Place> place)
            : id(id), weight(weight), place(place) {}

}