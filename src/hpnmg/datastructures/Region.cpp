#include "Region.h"

namespace hpnmg {
    Region Region::Empty() { return Region(PolytopeT::Empty()); }

    //region Region<->PolytopeT conversion
    Region::Region(const PolytopeT& polytope) : hPolytope(polytope) {}
    Region::operator const PolytopeT &() const { return hPolytope; }
    //endregion Region<->PolytopeT conversion
}
