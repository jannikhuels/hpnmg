#ifndef HPNMG_TRIANGULATION_H
#define HPNMG_TRIANGULATION_H

#include <CGAL/Delaunay_triangulation.h>
#include <CGAL/Epick_d.h>
#include <vector>
#include "ParametricLocationTree.h"

typedef CGAL::Triangulation<CGAL::Epick_d< CGAL::Dynamic_dimension_tag>> DT;
typedef CGAL::Triangulation_data_structure<CGAL::Epick_d< CGAL::Dynamic_dimension_tag>> DTDS;

namespace hpnmg {

    class Triangulation {

    private:
        std::vector<Region> t;
        std::vector<Region> t_split;
    public:

        static std::vector<Region> create(const Region &region);
        static std::vector<Region> create(const ParametricLocationTree::Node &node);

        std::vector<Region> getObjects();
        std::vector<Region> getSplitVertical();
        std::vector<std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>> getBounds();
    };
}


#endif //HPNMG_TRIANGULATION_H