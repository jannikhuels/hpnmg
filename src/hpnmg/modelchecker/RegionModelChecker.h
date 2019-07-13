#ifndef HPNMG_REGIONMODELCHECKER_H
#define HPNMG_REGIONMODELCHECKER_H

#include <memory>
#include <utility>
#include <vector>

#include "datastructures/HybridPetrinet.h"
#include "datastructures/STDPolytope.h"
#include "modelchecker/Formula.h"
#include "modelchecker/Until.h"
#include "ParametricLocationTree.h"

namespace hpnmg {
    class RegionModelChecker {
    public:
        RegionModelChecker(HybridPetrinet hpng, double maxTime);

        std::pair<double, double> satisfies(const Formula &formula, double atTime);

    private:
        STDPolytope<double> cfml(const ParametricLocationTree::Node& node, const string& placeIndex, int value);
        STDPolytope<double> dfml(const ParametricLocationTree::Node &node, const string& placeIndex, int value);
        std::vector<STDPolytope<double>> conj(const std::vector<STDPolytope<double>>& a, const std::vector<STDPolytope<double>>& b);
        std::vector<STDPolytope<double>> neg(const ParametricLocationTree::Node & node, const std::vector<STDPolytope<double>>& subSat);
        std::vector<STDPolytope<double>::Polytope> until(const ParametricLocationTree::Node& node, const Until& formula, double atTime);

        std::vector<STDPolytope<double>> satisfiesHandler(const ParametricLocationTree::Node& node, const Formula &formula, double atTime);

        std::shared_ptr<HybridPetrinet> hpng;
        ParametricLocationTree plt;
    };
}


#endif //HPNMG_REGIONMODELCHECKER_H
