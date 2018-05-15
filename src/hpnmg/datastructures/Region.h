#pragma once

#include "representations/GeometricObject.h"

namespace hpnmg{
    typedef hypro::HPolytope<double> Region;
    typedef std::vector<carl::Interval<double>> Intervals;
};