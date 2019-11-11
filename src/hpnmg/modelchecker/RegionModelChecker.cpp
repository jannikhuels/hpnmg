#include "RegionModelChecker.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "datastructures/STDPolytope.h"
#include "modelchecker/Conjunction.h"
#include "modelchecker/ContinuousAtomicProperty.h"
#include "modelchecker/DiscreteAtomicProperty.h"
#include "modelchecker/Formula.h"
#include "modelchecker/Negation.h"
#include "modelchecker/Until.h"
#include "ParseHybridPetrinet.h"
#include "ProbabilityCalculator.h"
#include "util/logging/Logging.h"
#include "util/statistics/Statistics.h"

namespace hpnmg {
    RegionModelChecker::RegionModelChecker(HybridPetrinet hpng, double maxTime) :
        hpng(std::make_shared<HybridPetrinet>(hpng)),
        plt(*ParseHybridPetrinet{}.parseHybridPetrinet(this->hpng, maxTime))
    {
        INFOLOG("hpnmg.RegionModelChecker", "[Number of Model dimensions]:" << this->plt.getDimension())
    }

    std::pair<double, double> RegionModelChecker::satisfies(const Formula &formula, double atTime) {
        double probability = 0.0;
        double error = 0.0;
        auto calculator = ProbabilityCalculator();

        for (auto &node : this->plt.getCandidateLocationsForTime(atTime)) {
            COUNT_STATS("MODELCHECKING_OVERALL")
            START_BENCHMARK_OPERATION("MODELCHECKING_OVERALL")
            TRACELOG("hpnmg.RegionModelChecker", "[Location " << node.getNodeID() << "]: Computing STD region.")
            node.computeRegion(this->plt);

            TRACELOG("hpnmg.RegionModelChecker", "Done. Dimensions: (vertex dim, effective dim) = (" << node.getRegion().dimension() << ", " << node.getRegion().effectiveDimension() << ")")

            TRACELOG("hpnmg.RegionModelChecker", "[Location " << node.getNodeID() << "]: Computing sat.")
            const auto& sat = this->satisfiesHandler(node, formula, atTime);
            TRACELOG("hpnmg.RegionModelChecker", "[Location " << node.getNodeID() << "]: Computing time slice of " << sat.size() << " polytopes.")
            std::vector<hypro::HPolytope<double>> integrationDomains{};
            // Add all result polytopes intersected with the check-time hyperplane to sat
            std::transform(sat.begin(), sat.end(), std::back_inserter(integrationDomains), [atTime](const STDPolytope<mpq_class> &region) {
                return convertHPolytope(region.timeSlice(atTime));
            });
            integrationDomains.erase(
                std::remove_if(integrationDomains.begin(), integrationDomains.end(), [](const auto &region) { return region.empty(); }),
                integrationDomains.end()
            );

            TRACELOG("hpnmg.RegionModelChecker", "[Location " << node.getNodeID() << "]: Integrating over " << integrationDomains.size() << " time slices.")

            if (integrationDomains.size())
                std::cout << STDPolytope<double>(integrationDomains[0]) << std::endl;

            double nodeError = 0.0;
            probability += calculator.getProbabilityForUnionOfPolytopesUsingDirectMonteCarlo(
                integrationDomains,
                this->plt.getDistributionsNormalized(),
                3,
                50000,
                nodeError
            ) * node.getParametricLocation().getAccumulatedProbability();
            STOP_BENCHMARK_OPERATION("MODELCHECKING_OVERALL")
            TRACELOG("hpnmg.RegionModelChecker", "[Location " << node.getNodeID() << "]: Running total probability: " << probability)
            error += nodeError;
        }

        return {probability, error};
    }

    std::vector<STDPolytope<mpq_class>> RegionModelChecker::satisfiesHandler(const ParametricLocationTree::Node& node, const Formula &formula, double atTime) {
        switch (formula.getType()) {
            case Formula::Type::Conjunction:
                return this->conj(node, *formula.getConjunction(), atTime);
            case Formula::Type::ContinuousAtomicProperty: {
                auto result = std::vector<STDPolytope<mpq_class>>();
                auto region = this->cfml(node, formula.getContinuousAtomicProperty()->place, formula.getContinuousAtomicProperty()->value);
                if (!region.empty())
                    result.push_back(region);
                return result;
            }
            case Formula::Type::DiscreteAtomicProperty: {
                auto result = std::vector<STDPolytope<mpq_class>>();
                auto region = this->dfml(node, formula.getDiscreteAtomicProperty()->place, formula.getDiscreteAtomicProperty()->value);
                if (!region.empty())
                    result.push_back(region);
                return result;
            }
            case Formula::Type::False:
                return {};
            case Formula::Type::Negation:
                return this->neg(node, formula.getNegation()->formula, atTime);
            case Formula::Type::True:
                return {STDPolytope<mpq_class>(node.getRegion())};
            case Formula::Type::Until: {
                if (this->withinUntil)
                    throw std::invalid_argument("RegionModelChecker encountered nested Until formulae.");

                this->untilCache.erase(this->untilCache.begin(), this->untilCache.end());

                this->withinUntil = true;
                const auto result = this->until(node, *formula.getUntil(), atTime);
                this->withinUntil = false;

                return result;
            }
        }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
        throw std::logic_error("unhandled formula type in RegionModelChecker::satisfiesHandler");
#pragma clang diagnostic pop
    }

    //TODO this checks x_p <= u. Should I change this?
    STDPolytope<mpq_class> RegionModelChecker::cfml(const ParametricLocationTree::Node& node, const std::string& placeId, int value, bool negate) {
        COUNT_STATS("MODELCHECKING_CONTINUOUS")
        START_BENCHMARK_OPERATION("MODELCHECKING_CONTINUOUS")
        const auto &continuousPlaces = this->hpng->getContinuousPlaces();
        if (continuousPlaces.count(placeId) == 0)
            throw std::invalid_argument(std::string("no such continuous place in model: ") + placeId);

        auto placeOffset = std::distance(continuousPlaces.begin(), continuousPlaces.find(placeId));

        STDPolytope<double> candidateRegion = node.getRegion();

        double drift = node.getParametricLocation().getDrift()[placeOffset];
        STDPolytope<double> regionIntersected = STDiagram::intersectRegionForContinuousLevel(
            candidateRegion,
            node.getParametricLocation().getSourceEvent().getGeneralDependenciesNormed(),
            node.getParametricLocation().getContinuousMarkingNormed()[placeOffset],
            drift,
            value,
            negate
        );
        TRACELOG("hpnmg.RegionModelChecker", "post level intersection: (vertex dim, effective dim) = (" << regionIntersected.dimension() << ", " << regionIntersected.effectiveDimension() << ")")
        STOP_BENCHMARK_OPERATION("MODELCHECKING_CONTINUOUS")
        return STDPolytope<mpq_class>(regionIntersected);
    }

    STDPolytope<mpq_class> RegionModelChecker::dfml(const ParametricLocationTree::Node &node, const std::string& placeId, int value, bool negate) {
        COUNT_STATS("MODELCHECKING_DISCRETE")
        START_BENCHMARK_OPERATION("MODELCHECKING_DISCRETE")
        const auto &discretePlaces = this->hpng->getDiscretePlaces();
        if (discretePlaces.count(placeId) == 0)
            throw std::invalid_argument(std::string("no such discrete place in model: ") + placeId);

        auto placeOffset = std::distance(discretePlaces.begin(), discretePlaces.find(placeId));
        const bool satisfied = node.getParametricLocation().getDiscreteMarking().at(placeOffset) == value;
        STOP_BENCHMARK_OPERATION("MODELCHECKING_DISCRETE")
        if (satisfied != negate)
            return STDPolytope<mpq_class>(node.getRegion());
        else
            return STDPolytope<mpq_class>::Empty(node.getRegion().dimension());
    }

    std::vector<STDPolytope<mpq_class>> RegionModelChecker::conj(const ParametricLocationTree::Node& node, const Conjunction& conj, double atTime) {
         // In general, sat(psi AND phi) equals to the pairwise intersection of sat(psi) and sat(phi). However, for
         // trivial sub-formulae we can skip the intersection:
         //    - sat(true AND phi) -> sat(phi)
         //    - sat(false AND phi) -> empty set
        COUNT_STATS("MODELCHECKING_CONJUNCTION")
        START_BENCHMARK_OPERATION("MODELCHECKING_CONJUNCTION")
         const auto lhsTrivial = conj.left.isTrivial();
        const auto rhsTrivial = conj.right.isTrivial();

        std::vector<STDPolytope<mpq_class>> sat;
        if (lhsTrivial.first && rhsTrivial.first) { // Both formulae are trivial -> return full region or empty region
            if (lhsTrivial.second.getType() == Formula::Type::True && rhsTrivial.second.getType() == Formula::Type::True)
                sat = {STDPolytope<mpq_class>(node.getRegion())};
            else
                sat = {};
        } else if(!lhsTrivial.first && !rhsTrivial.first) { // Neither formula is trivial -> evaluate both and intersect
            const auto leftSat = this->satisfiesHandler(node, conj.left, atTime);
            const auto rightSat = this->satisfiesHandler(node, conj.right, atTime);
            sat = {};
            sat.reserve(leftSat.size() * rightSat.size());
            for(const STDPolytope<mpq_class>& ai : leftSat) {
                for (const STDPolytope<mpq_class>& bi : rightSat) {
                    STDPolytope<mpq_class> res = ai.intersect(bi);
                    if (!res.empty()) {
                        sat.push_back(res);
                    }
                }
            }
            sat.shrink_to_fit();
        } else { // One is trivial -> return sat(other) or empty region
            const auto& trivialFormula = lhsTrivial.first ? lhsTrivial : rhsTrivial;
            const auto& nonTrivialFormula = lhsTrivial.first ? rhsTrivial : lhsTrivial;

            if (trivialFormula.second.getType() == Formula::Type::True) {
                STOP_BENCHMARK_OPERATION("MODELCHECKING_CONJUNCTION")
                sat = this->satisfiesHandler(node, nonTrivialFormula.second, atTime);
            }
            else
                sat = {};
        }
        STOP_BENCHMARK_OPERATION("MODELCHECKING_CONJUNCTION")
        return sat;
    }

    std::vector<STDPolytope<mpq_class>> RegionModelChecker::neg(const ParametricLocationTree::Node& node, const Negation& formula, double atTime) {
        COUNT_STATS("MODELCHECKING_NEGATION")
        START_BENCHMARK_OPERATION("MODELCHECKING_NEGATION")
        const STDPolytope<mpq_class>& nodeRegion = STDPolytope<mpq_class>(node.getRegion());

        const auto innerFormula = formula.formula;
        const auto innerTrivial = innerFormula.isTrivial();
        if (innerTrivial.first) {
            if (innerTrivial.second.getType() == Formula::Type::True)
                return {};
            else
                return {nodeRegion};
        }

        // Even if the formula is non-trivial, we might still skip the expensive computation of the set-differences.
        switch (innerFormula.getType()) {
            case Formula::Type::ContinuousAtomicProperty: {
                const auto cap = innerFormula.getContinuousAtomicProperty();
                STOP_BENCHMARK_OPERATION("MODELCHECKING_NEGATION")
                return {this->cfml(node, cap->place, cap->value, true)};
            }
            case Formula::Type::DiscreteAtomicProperty: {
                const auto dap = innerFormula.getDiscreteAtomicProperty();
                STOP_BENCHMARK_OPERATION("MODELCHECKING_NEGATION")
                return {this->dfml(node, dap->place, dap->value, true)};
            }
            case Formula::Type::Negation: {
                STOP_BENCHMARK_OPERATION("MODELCHECKING_NEGATION")
                return this->satisfiesHandler(node, innerFormula.getNegation()->formula, atTime);
            }
            // These formulae cannot be negated easily in general:
            case Formula::Type::Conjunction:
            case Formula::Type::Until:
                break;
            default:
                throw std::logic_error("unhandled non-trivial formula type in RegionModelChecker::neg");
        }

        const auto subSat = this->satisfiesHandler(node, innerFormula, atTime);
        // If the nested satisfaction set is empty, the whole region satisfies the negation
        if (subSat.empty()) {
            STOP_BENCHMARK_OPERATION("MODELCHECKING_NEGATION")
            return {nodeRegion};
        }

        std::vector<STDPolytope<mpq_class>> sat;
        for (const auto& polytope : subSat) {
            auto negationResult = nodeRegion.setDifference(polytope);

            // If the satisfaction set is empty, the negations make up the whole satisfaction set.
            if (sat.empty()) {
                std::move(negationResult.begin(), negationResult.end(), std::back_inserter(sat));
                continue;
            }

            // In order to merge the satisfaction set with the new negations we need compute all pairwise intersections
            auto intersections = std::vector<STDPolytope<mpq_class>>();
            intersections.reserve(sat.size() * negationResult.size());
            for (const auto& negatedPolytope : negationResult) {
                std::transform(sat.begin(), sat.end(), std::back_inserter(intersections), [&negatedPolytope](const STDPolytope<mpq_class>& satPolytope) {
                    return negatedPolytope.intersect(satPolytope);
                });
            }

            // Get rid off empty polytopes
            intersections.erase(
                std::remove_if(intersections.begin(), intersections.end(), [](STDPolytope<mpq_class> p) { return p.empty(); }),
                intersections.end()
            );
            sat = intersections;
        }
        STOP_BENCHMARK_OPERATION("MODELCHECKING_NEGATION")
        return sat;
    }

    std::vector<STDPolytope<mpq_class>> RegionModelChecker::until(const ParametricLocationTree::Node& node, const Until& formula, double atTime) {
        COUNT_STATS("MODELCHECKING_UNTIL")
        START_BENCHMARK_OPERATION("MODELCHECKING_UNTIL")
        auto sat = this->untilHandler(node, formula, atTime);

        TRACELOG("hpnmg.RegionModelChecker.Until", "sat(until) computed. Intersecting it with the node's region.")

        // Construct the upper and lower boundary halfspaces defined by the check-time and the check-time plus until-time.
        hypro::vector_t<mpq_class> timeVectorUp = hypro::vector_t<mpq_class>::Zero(node.getRegion().dimension());
        timeVectorUp[timeVectorUp.size() - 1] = 1;
        const auto upperLimit = hypro::Halfspace<mpq_class>(timeVectorUp, atTime + formula.withinTime);
        const auto lowerLimit = hypro::Halfspace<mpq_class>(-timeVectorUp, atTime);
        // Construct the proper bounding box from the node's region and the time planes.
        auto fullRegion = STDPolytope<mpq_class>(node.getRegion());
        fullRegion.insert(upperLimit);
        fullRegion.insert(lowerLimit);

        std::transform(sat.begin(), sat.end(), sat.begin(), [&fullRegion](auto&& region) { return region.intersect(fullRegion); });
        sat.erase(std::remove_if(sat.begin(), sat.end(), [](const auto &region) { return region.empty(); }), sat.end());
        STOP_BENCHMARK_OPERATION("MODELCHECKING_UNTIL")
        return sat;
    }

    std::vector<STDPolytope<mpq_class>> RegionModelChecker::untilHandler(const ParametricLocationTree::Node& node, const Until& formula, double atTime) {
        auto cached = this->untilCache.find(node.getNodeID());
        if (cached != this->untilCache.end())
            return cached->second;
        COUNT_STATS("MODELCHECKING_UNTIL_HANDLER")
        // Construct the upper and lower boundary halfspaces defined by the check-time and the check-time plus until-time.
        hypro::vector_t<mpq_class> timeVectorUp = hypro::vector_t<mpq_class>::Zero(node.getRegion().dimension());
        timeVectorUp[timeVectorUp.size() - 1] = 1;
        const auto upperLimit = hypro::Halfspace<mpq_class>(timeVectorUp, atTime + formula.withinTime);
        const auto lowerLimit = hypro::Halfspace<mpq_class>(-timeVectorUp, atTime);

        // Make sure that this node is even reachable within the formula's time frame
        auto fullRegion = STDPolytope<mpq_class>(node.getRegion());
        TRACELOG("hpnmg.RegionModelChecker.Until", "Dimension: [" << fullRegion.effectiveDimension() << ", " << fullRegion.dimension() << "]")

        // Check if the current region is a vanishing region, and only consider the children
        if (fullRegion.effectiveDimension() < fullRegion.dimension()) {
            std::vector<STDPolytope<mpq_class>> childrenSat{};
            for (auto& childNode : this->plt.getChildNodes(node)) {
                TRACELOG("hpnmg.RegionModelChecker.Until", "Until @ Node " << node.getNodeID() << ": Recursively checking Node " << childNode.getNodeID())
                childNode.computeRegion(this->plt);
                // untilHandler returns downwards extended polytopes
                auto childSat = this->untilHandler(childNode, formula, atTime);
                std::move(childSat.begin(), childSat.end(), std::back_inserter(childrenSat));
            }
            this->untilCache.insert({node.getNodeID(), childrenSat});
            return childrenSat;
        }

        fullRegion.insert(upperLimit);
        fullRegion.insert(lowerLimit);
        if (fullRegion.empty()){
            this->untilCache.insert({node.getNodeID(), {}});
            return {};
        }

        std::vector<STDPolytope<mpq_class>> eventualSat{};
        //region Gather all polytopes that most certainly fulfill `formula` within the time frame.
        // The polytopes in the current region that fulfill `formula.goal`
        auto regionSat = this->satisfiesHandler(node, formula.goal, atTime);
        START_BENCHMARK_OPERATION("MODELCHECKING_UNTIL_HANDLER_GOAL")
        std::transform(regionSat.begin(), regionSat.end(), regionSat.begin(), [&upperLimit](STDPolytope<mpq_class> sat) {
            sat.insert(upperLimit);
            TRACELOG("hpnmg.RegionModelChecker.Until", "Extending member of sat(goal) downwards.")
            return STDPolytope<mpq_class>(sat.extendDownwards());
        });
        regionSat.erase(std::remove_if(regionSat.begin(), regionSat.end(), [](const auto& region) { return region.empty(); }), regionSat.end());
        std::move(regionSat.begin(), regionSat.end(), std::back_inserter(eventualSat));
        //endregion Gather all polytopes that most certainly fulfill `formula`.
        STOP_BENCHMARK_OPERATION("MODELCHECKING_UNTIL_HANDLER_GOAL")
        TRACELOG("hpnmg.RegionModelChecker.Until", "Until @ Node " << node.getNodeID() << ": Done.")

        // All polytopes that do satisfy or do not satisfy the until formula inside this region
        std::vector<STDPolytope<mpq_class>> knownSat(eventualSat);
        // Get the polytopes where the until is "unfulfilled" immediately. The downward extension is needed multiple
        // times, so it is calculated and cached here already.
        START_BENCHMARK_OPERATION("MODELCHECKING_UNTIL_HANDLER_INSIDE")
        auto deadRegions = std::vector<std::pair<STDPolytope<mpq_class>, STDPolytope<mpq_class>::Polytope>>();
        const auto& preTrivial = formula.pre.isTrivial();
        if (preTrivial.first) {
            if (preTrivial.second.getType() == Formula::Type::True) {
                TRACELOG("hpnmg.RegionModelChecker.Until", "Until @ Node " << node.getNodeID() << ": Precondition is trivially true. No need to compute dead regions.")
                deadRegions = {};
            } else {
                TRACELOG("hpnmg.RegionModelChecker.Until", "Until @ Node " << node.getNodeID() << ": Precondition is trivially false. Returning early with sat(goal).")
                this->untilCache.insert({node.getNodeID(), regionSat});
                return regionSat;
            }
        } else {
            TRACELOG("hpnmg.RegionModelChecker.Until",  "Until @ Node " << node.getNodeID() << ": Precondition is non-trivial. Computing sat(dead).")

            auto deadFormula = Formula(std::make_shared<Conjunction>(
                Formula(std::make_shared<Negation>(formula.pre)),
                Formula(std::make_shared<Negation>(formula.goal))
            ));

            for (const auto& deadRegion : this->satisfiesHandler(node, deadFormula, atTime)) {
                TRACELOG("hpnmg.RegionModelChecker.Until",  "Done. Extending member of sat(dead) downwards.")
                const auto deadRegionDownwardsExtension = deadRegion.extendDownwards();
                deadRegions.emplace_back(deadRegion, deadRegionDownwardsExtension);
                knownSat.push_back(STDPolytope<mpq_class>(deadRegionDownwardsExtension));
            }
            TRACELOG("hpnmg.RegionModelChecker.Until",  "Done.")
        }
        STOP_BENCHMARK_OPERATION("MODELCHECKING_UNTIL_HANDLER_INSIDE")

        // Check if there is a part of the region left, which needs the processing in the child locations.
        START_BENCHMARK_OPERATION("MODELCHECKING_UNTIL_HANDLER_SET_DIFFERENCE")
        if (!fullRegion.setDifference(knownSat).empty()) {
            STOP_BENCHMARK_OPERATION("MODELCHECKING_UNTIL_HANDLER_SET_DIFFERENCE")
            // The polytopes resulting from recursively handled child locations
            for (auto& childNode : this->plt.getChildNodes(node)) {
                childNode.computeRegion(this->plt);
                // untilHandler returns downwards extended polytopes
                auto childSat = this->untilHandler(childNode, formula, atTime);
                std::move(childSat.begin(), childSat.end(), std::back_inserter(eventualSat));
            }
        } else {
            STOP_BENCHMARK_OPERATION("MODELCHECKING_UNTIL_HANDLER_SET_DIFFERENCE")
        }

        START_BENCHMARK_OPERATION("MODELCHECKING_UNTIL_HANDLER_SATISFIED")
        auto sat = std::vector<STDPolytope<mpq_class>>();
        for (const auto& goalRegion : eventualSat) {
            std::vector<STDPolytope<mpq_class>> regionsToRemove{};
            regionsToRemove.reserve(deadRegions.size());
            // Every dead region below the goal region must be subtracted from the extended goal region. More specific,
            // the extended dead region needs to be subtracted.
            for (const auto& otherRegion : deadRegions) {
                // Since all satisfaction polytopes are convex and disjoint: if polytope A intersects with the
                // downward-extension of polytope B, then A must be (at least partially) directly below B but nowhere
                // directly above B.
                TRACELOG("hpnmg.RegionModelChecker.Until",  "Checking if member of sat(dead) needs to be removed from extended member of sat(goal).")
                if (!STDPolytope<mpq_class>(goalRegion).intersect(otherRegion.first).empty())
                    regionsToRemove.emplace_back(STDPolytope<mpq_class>(otherRegion.second));
            }

            TRACELOG("hpnmg.RegionModelChecker.Until",  "Removing all " << regionsToRemove.size() << " dead members from extended goal member.")
            auto goalSat = goalRegion.setDifference(regionsToRemove);
            goalSat.erase(std::remove_if(goalSat.begin(), goalSat.end(), [](const auto& region) { return region.empty(); }), goalSat.end());
            std::move(goalSat.begin(), goalSat.end(), std::back_inserter(sat));
        }
        STOP_BENCHMARK_OPERATION("MODELCHECKING_UNTIL_HANDLER_SATISFIED")
        TRACELOG("hpnmg.RegionModelChecker.Until",  "Computed " << sat.size() << " sat regions.")

        this->untilCache.insert({node.getNodeID(), sat});
        return sat;
    }
}
