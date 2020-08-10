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

        ParametricLocationTree getPlt();

    private:
        STDPolytope<mpq_class> cfml(const ParametricLocationTree::Node& node, const string& placeIndex, int value, bool negate = false);
        STDPolytope<mpq_class> dfml(const ParametricLocationTree::Node &node, const string& placeIndex, int value, bool negate = false);
        std::vector<STDPolytope<mpq_class>> conj(const ParametricLocationTree::Node &node, const Conjunction& conj, double atTime);
        std::vector<STDPolytope<mpq_class>> neg(const ParametricLocationTree::Node & node, const Negation& formula, double atTime);

        /**
         * Computes the satisfaction set `sat(formula.pre Until formula.goal, atTime)` for the given node.
         *
         * Each element of the satisfaction set has its time dimension already limited to [atTime, atTime + formula.withinTime]
         *
         * @param node
         * @param formula
         * @param atTime
         * @return A vector of polytopes whose union represents the satisfaction set `sat(formula, atTime)` in the
         *         provided node's region. Each polytope has been intersected with the time-halfspaces induced by
         *         `atTime` and `atTime + formula.withinTime`.
         */
        std::vector<STDPolytope<mpq_class>> until(const ParametricLocationTree::Node& node, const Until& formula, double atTime);

        /**
         * Recursive utility function for the until-computation.
         *
         * Most notably, the returned polytopes are still extended downwards, that is, they are not necessarily within
         * their respective region but instead may extend below that. If the polytopes were constrained before returning
         * them, the caller would have to extend them immediately anyway which is an expensive operation.
         *
         * Utilizes RegionModelChecker::untilCache to memoize the satisfaction sets for each location.
         *
         * @param node
         * @param formula
         * @param atTime
         * @return A vector of polytopes whose union represents the satisfaction set `sat(formula, atTime)` for the
         *         provided node's region. However, each polytope is extended downwards (see STDPolytope::extendDownwards()).
         *         So in order to get the actual satisfaction polytopes for the node, you need to intersect them with
         *         the node's region.
         */
        std::vector<STDPolytope<mpq_class>> untilHandler(const ParametricLocationTree::Node& node, const Until& formula, double atTime);

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
