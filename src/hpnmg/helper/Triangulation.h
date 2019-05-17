#ifndef HPNMG_TRIANGULATION_H
#define HPNMG_TRIANGULATION_H

#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/Epick_d.h>
#include <vector>
#include "ParametricLocationTree.h"

typedef CGAL::Triangulation<CGAL::Epick_d< CGAL::Dynamic_dimension_tag>> DT;

namespace hpnmg {

    class Triangulation {
    public:
        static std::vector<Region> create(const Region &region);
        static std::vector<Region> create(const ParametricLocationTree::Node &node);
    };
}


#endif //HPNMG_TRIANGULATION_H