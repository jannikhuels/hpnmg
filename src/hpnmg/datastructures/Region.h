#pragma once

#include "representations/GeometricObject.h"

namespace hpnmg {
    typedef std::vector<carl::Interval<double>> Intervals;

    class Region {
    public:
        using PolytopeT = hypro::HPolytope<double>;

        static Region Empty();

        Region() = default;

        //region Region<->PolytopeT conversion
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
        Region(PolytopeT polytope);
        operator const PolytopeT &() const;
#pragma clang diagnostic pop
        //endregion Region<->PolytopeT conversion

        PolytopeT hPolytope = PolytopeT();
    };
};