#ifndef HPNMG_REGIONMODELCHECKER_H
#define HPNMG_REGIONMODELCHECKER_H

#include <memory>
#include <utility>
#include <vector>

#include <gmpxx.h>

#include "datastructures/HybridPetrinet.h"
#include "datastructures/STDPolytope.h"
#include "modelchecker/Formula.h"
#include "modelchecker/Negation.h"
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
        STDPolytope<mpq_class> cfml(const ParametricLocationTree::Node& node, const string& placeIndex, int value, bool negate = false);
        STDPolytope<mpq_class> dfml(const ParametricLocationTree::Node &node, const string& placeIndex, int value, bool negate = false);
        std::vector<STDPolytope<mpq_class>> conj(const std::vector<STDPolytope<mpq_class>>& a, const std::vector<STDPolytope<mpq_class>>& b);
        std::vector<STDPolytope<mpq_class>> neg(const ParametricLocationTree::Node & node, const Negation& formula, double atTime);
        std::vector<STDPolytope<mpq_class>> until(const ParametricLocationTree::Node& node, const Until& formula, double atTime);

        std::vector<STDPolytope<mpq_class>> satisfiesHandler(const ParametricLocationTree::Node& node, const Formula &formula, double atTime);

        std::shared_ptr<HybridPetrinet> hpng;
        ParametricLocationTree plt;

        /**
         * Indicates whether the checker is currently checking a (sub) formula of an encountered until.
         *
         * Currently, the RegionModelChecker does not support nested untils so we need a way to detect such formulae.
         */
        bool withinUntil = false;
        std::unordered_map<NODE_ID, std::vector<STDPolytope<mpq_class>>> untilCache;
    };
}


#endif //HPNMG_REGIONMODELCHECKER_H
