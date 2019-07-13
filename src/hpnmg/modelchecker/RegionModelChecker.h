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

        /**
         * Compute the probability of <code>formula</code> holding at time <code>atTime</code> in the model associated
         * with the RegionModelChecker instance. The resulting pair <code>(prob, err)</code> denotes said probability
         * and an absolute error estimate.
         *
         * The error is only an estimate because Monte Carlo integration, which is used to compute the probability, is a
         * probabilistic algorithm.
         *
         * Important: <code>formula</code> **must not** contain nested until formulae. This is not supported as of now.
         *
         * @param formula A <code>Formula</code> instance to be checked against the stored HPnG. Must not contain nested
         *                <code>Until</code> instances.
         * @param atTime The point in time at which the validity of the formula is to be checked.
         * @return A pair of doubles denoting the probability of <code>formula</code> holding at time <code>atTime</code>
         *         and the absolute error. Note that this error is more of an estimate because a Monte Carlo integration
         *         is used.
         *
         */
        std::pair<double, double> satisfies(const Formula &formula, double atTime);

    private:
        STDPolytope<double> cfml(const ParametricLocationTree::Node& node, const string& placeIndex, int value);
        STDPolytope<double> dfml(const ParametricLocationTree::Node &node, const string& placeIndex, int value);
        std::vector<STDPolytope<double>> conj(const std::vector<STDPolytope<double>>& a, const std::vector<STDPolytope<double>>& b);
        std::vector<STDPolytope<double>> neg(const ParametricLocationTree::Node & node, const std::vector<STDPolytope<double>>& subSat);
        std::vector<STDPolytope<double>> until(const ParametricLocationTree::Node& node, const Until& formula, double atTime);

        std::vector<STDPolytope<double>> satisfiesHandler(const ParametricLocationTree::Node& node, const Formula &formula, double atTime);

        std::shared_ptr<HybridPetrinet> hpng;
        ParametricLocationTree plt;

        /**
         * Indicates whether the checker is currently checking a (sub) formula of an encountered until.
         *
         * Currently, the RegionModelChecker does not support nested untils so we need a way to detect such formulae.
         */
        bool withinUntil = false;
        std::unordered_map<NODE_ID, std::vector<STDPolytope<double>>> untilCache;
    };
}


#endif //HPNMG_REGIONMODELCHECKER_H
