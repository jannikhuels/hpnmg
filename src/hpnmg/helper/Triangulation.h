#ifndef HPNMG_TRIANGULATION_H
#define HPNMG_TRIANGULATION_H

#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/Epick_d.h>
#include <vector>
#include "ParametricLocationTree.h"

typedef CGAL::Triangulation<CGAL::Epick_d< CGAL::Dynamic_dimension_tag>> DT;

namespace hpnmg {

    class Triangulation {

    private:
        std::vector<Region> t;
        std::vector<Region> t_split;
    public:
        Triangulation(const ParametricLocationTree::Node &node);
        Triangulation(const Region &node);

        std::vector<Region> getObjects();
        std::vector<Region> getSplitVertical();
    };
}


#endif //HPNMG_TRIANGULATION_H