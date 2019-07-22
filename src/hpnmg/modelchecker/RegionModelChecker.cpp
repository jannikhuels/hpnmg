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

namespace hpnmg {
    RegionModelChecker::RegionModelChecker(HybridPetrinet hpng, double maxTime) :
        hpng(std::make_shared<HybridPetrinet>(hpng)),
        plt(*ParseHybridPetrinet{}.parseHybridPetrinet(this->hpng, maxTime))
    {}

    std::pair<double, double> RegionModelChecker::satisfies(const Formula &formula, double atTime) {
        double probability = 0.0;
        double error = 0.0;
        auto calculator = ProbabilityCalculator();

        for (auto &node : this->plt.getCandidateLocationsForTime(atTime)) {
            node.computeRegion(this->plt);
            std::cout << "[Location " << node.getNodeID() << "]: Computing sat." << std::endl;
            const auto& sat = this->satisfiesHandler(node, formula, atTime);

            std::cout << "[Location " << node.getNodeID() << "]: Computing time slice of " << sat.size() << " polytopes." << std::endl;
            std::vector<hypro::HPolytope<double>> integrationDomains{};
            // Add all result polytopes intersected with the check-time hyperplane to sat
            std::transform(sat.begin(), sat.end(), std::back_inserter(integrationDomains), [atTime](const STDPolytope<mpq_class> &region) {
                return convertHPolytope(region.timeSlice(atTime));
            });
            integrationDomains.erase(
                std::remove_if(integrationDomains.begin(), integrationDomains.end(), [](const auto &region) { return region.empty(); }),
                integrationDomains.end()
            );

            std::cout << "[Location " << node.getNodeID() << "]: Integrating over " << integrationDomains.size() << " time slices." << std::endl;
            double nodeError = 0.0;
            probability += calculator.getProbabilityForUnionOfPolytopesUsingMonteCarlo(
                integrationDomains,
                this->plt.getDistributionsNormalized(),
                3,
                50000,
                nodeError
            );
            std::cout << "[Location " << node.getNodeID() << "]: Running total probability: " << probability << std::endl;
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

        return STDPolytope<mpq_class>(regionIntersected);
    }

    STDPolytope<mpq_class> RegionModelChecker::dfml(const ParametricLocationTree::Node &node, const std::string& placeId, int value, bool negate) {
        const auto &discretePlaces = this->hpng->getDiscretePlaces();
        if (discretePlaces.count(placeId) == 0)
            throw std::invalid_argument(std::string("no such discrete place in model: ") + placeId);

        auto placeOffset = std::distance(discretePlaces.begin(), discretePlaces.find(placeId));
        const bool satisfied = node.getParametricLocation().getDiscreteMarking().at(placeOffset) == value;
        if (satisfied != negate)
            return STDPolytope<mpq_class>(node.getRegion());
        else
            return STDPolytope<mpq_class>::Empty();
    }

    std::vector<STDPolytope<mpq_class>> RegionModelChecker::conj(const ParametricLocationTree::Node& node, const Conjunction& conj, double atTime) {
         // In general, sat(psi AND phi) equals to the pairwise intersection of sat(psi) and sat(phi). However, for
         // trivial sub-formulae we can skip the intersection:
         //    - sat(true AND phi) -> sat(phi)
         //    - sat(false AND phi) -> empty set
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

            if (trivialFormula.second.getType() == Formula::Type::True)
                sat = this->satisfiesHandler(node, nonTrivialFormula.second, atTime);
            else
                sat = {};
        }

        return sat;
    }

    std::vector<STDPolytope<mpq_class>> RegionModelChecker::neg(const ParametricLocationTree::Node& node, const Negation& formula, double atTime) {
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
                return {this->cfml(node, cap->place, cap->value, true)};
            }
            case Formula::Type::DiscreteAtomicProperty: {
                const auto dap = innerFormula.getDiscreteAtomicProperty();
                return {this->dfml(node, dap->place, dap->value, true)};
            }
            case Formula::Type::Negation: {
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
        if (subSat.empty())
            return {nodeRegion};

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

        return sat;
    }

    std::vector<STDPolytope<mpq_class>> RegionModelChecker::until(const ParametricLocationTree::Node& node, const Until& formula, double atTime) {
        auto sat = this->untilHandler(node, formula, atTime);

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

        return sat;
    }

    std::vector<STDPolytope<mpq_class>> RegionModelChecker::untilHandler(const ParametricLocationTree::Node& node, const Until& formula, double atTime) {
        auto cached = this->untilCache.find(node.getNodeID());
        if (cached != this->untilCache.end())
            return cached->second;

        // Construct the upper and lower boundary halfspaces defined by the check-time and the check-time plus until-time.
        hypro::vector_t<mpq_class> timeVectorUp = hypro::vector_t<mpq_class>::Zero(node.getRegion().dimension());
        timeVectorUp[timeVectorUp.size() - 1] = 1;
        const auto upperLimit = hypro::Halfspace<mpq_class>(timeVectorUp, atTime + formula.withinTime);
        const auto lowerLimit = hypro::Halfspace<mpq_class>(-timeVectorUp, atTime);

        // Make sure that this node is even reachable within the formula's time frame
        auto fullRegion = STDPolytope<mpq_class>(node.getRegion());
        fullRegion.insert(upperLimit);
        fullRegion.insert(lowerLimit);
        if (fullRegion.empty()){
            this->untilCache.insert({node.getNodeID(), {}});
            return {};
        }

        std::vector<STDPolytope<mpq_class>> eventualSat{};
        //region Gather all polytopes that most certainly fulfill `formula` within the time frame.
        // 1. The polytopes resulting from recursively handled child locations
        for (auto& childNode : this->plt.getChildNodes(node)) {
            childNode.computeRegion(this->plt);
            // untilHandler returns downwards extended polytopes
            auto childSat = this->untilHandler(childNode, formula, atTime);
            std::move(childSat.begin(), childSat.end(), std::back_inserter(eventualSat));
        }
        // 2. The polytopes in the current region that fulfill `formula.goal`
        auto regionSat = this->satisfiesHandler(node, formula.goal, atTime);
        std::transform(regionSat.begin(), regionSat.end(), regionSat.begin(), [&upperLimit](STDPolytope<mpq_class> sat) {
            sat.insert(upperLimit);
            return STDPolytope<mpq_class>(sat.extendDownwards());
        });
        regionSat.erase(std::remove_if(regionSat.begin(), regionSat.end(), [](const auto& region) { return region.empty(); }), regionSat.end());
        std::move(regionSat.begin(), regionSat.end(), std::back_inserter(eventualSat));
        //endregion Gather all polytopes that most certainly fulfill `formula`.

        // Get the polytopes where the until is "unfulfilled" immediately. The downward extension is needed multiple
        // times, so it is calculated and cached here already.
        auto deadRegions = std::vector<std::pair<STDPolytope<mpq_class>, STDPolytope<mpq_class>::Polytope>>();
        const auto& preTrivial = formula.pre.isTrivial();
        if (preTrivial.first) {
            if (preTrivial.second.getType() == Formula::Type::True) {
                deadRegions = {};
            } else {
                this->untilCache.insert({node.getNodeID(), regionSat});
                return regionSat;
            }
        } else {
            auto deadFormula = Formula(std::make_shared<Conjunction>(
                Formula(std::make_shared<Negation>(formula.pre)),
                Formula(std::make_shared<Negation>(formula.goal))
            ));

            for (const auto& deadRegion : this->satisfiesHandler(node, deadFormula, atTime)) {
                deadRegions.emplace_back(deadRegion, deadRegion.extendDownwards());
            }
        }

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
                if (!STDPolytope<mpq_class>(goalRegion).intersect(otherRegion.first).empty())
                    regionsToRemove.emplace_back(STDPolytope<mpq_class>(otherRegion.second));
            }

            auto goalSat = goalRegion.setDifference(regionsToRemove);
            goalSat.erase(std::remove_if(goalSat.begin(), goalSat.end(), [](const auto& region) { return region.empty(); }), goalSat.end());
            std::move(goalSat.begin(), goalSat.end(), std::back_inserter(sat));
        }

        this->untilCache.insert({node.getNodeID(), sat});
        return sat;
    }
}
