#ifndef HPNMG_REGIONMODELCHECKER_H
#define HPNMG_REGIONMODELCHECKER_H

#include <memory>
#include <utility>
#include <vector>

#include "datastructures/HybridPetrinet.h"
#include "datastructures/STDPolytope.h"
#include "modelchecker/Formula.h"
#include "ParametricLocationTree.h"

namespace hpnmg {
    class RegionModelChecker {
    public:
        RegionModelChecker(HybridPetrinet hpng, double maxTime);

        std::pair<double, double> satisfies(const Formula &formula, double atTime);

    private:
        STDPolytope cfml(const ParametricLocationTree::Node& node, const string& placeIndex, int value);
        STDPolytope dfml(const ParametricLocationTree::Node &node, const string& placeIndex, int value);
        std::vector<STDPolytope> conj(std::vector<STDPolytope> a, std::vector<STDPolytope> b);
        std::vector<STDPolytope> neg(const ParametricLocationTree::Node & node, std::vector<STDPolytope> a);

        std::vector<STDPolytope> satisfiesHandler(const ParametricLocationTree::Node& node, const Formula &formula, double atTime);

        std::shared_ptr<HybridPetrinet> hpng;
        ParametricLocationTree plt;
    };
}


#endif //HPNMG_REGIONMODELCHECKER_H
