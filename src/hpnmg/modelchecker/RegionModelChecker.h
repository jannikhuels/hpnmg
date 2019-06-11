#ifndef HPNMG_REGIONMODELCHECKER_H
#define HPNMG_REGIONMODELCHECKER_H

#include <memory>
#include <vector>

#include "datastructures/HybridPetrinet.h"
#include "datastructures/Region.h"
#include "modelchecker/Formula.h"
#include "ParametricLocationTree.h"

namespace hpnmg {
    class RegionModelChecker {
    public:
        RegionModelChecker(HybridPetrinet hpng, double maxTime);

        double satisfies(const Formula &formula, double atTime);

    private:
        Region dfml(const ParametricLocationTree::Node &node, const string& placeIndex, int value);
        std::vector<Region> conj(std::vector<Region> a, std::vector<Region> b);
        std::vector<Region> satisfiesHandler(const Formula &formula, double atTime);

        std::shared_ptr<HybridPetrinet> hpng;
        ParametricLocationTree plt;
    };
}


#endif //HPNMG_REGIONMODELCHECKER_H
