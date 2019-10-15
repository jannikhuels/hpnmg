#ifndef HPNMG_TRIANGULATION_H
#define HPNMG_TRIANGULATION_H

#include "datastructures/Simplex.h"

#include <representations/GeometricObject.h>

#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/Epick_d.h>

#include <vector>

typedef CGAL::Triangulation<CGAL::Epick_d< CGAL::Dynamic_dimension_tag>> DT;

namespace hpnmg {

    class Triangulation {
    public:
        static std::vector<Simplex> create(const hypro::HPolytope<double> &p);
    };
}

#endif //HPNMG_TRIANGULATION_H