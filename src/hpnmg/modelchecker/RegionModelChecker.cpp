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
    {
        plt.updateRegions();
    }

    std::pair<double, double> RegionModelChecker::satisfies(const Formula &formula, double atTime) {
        auto sat = std::vector<hypro::HPolytope<double>>();

        for (const auto &node : this->plt.getCandidateLocationsForTime(atTime)) {
            const auto &result = this->satisfiesHandler(node, formula, atTime);

            // Add all result polytopes intersected with the check-time hyperplane to sat
            std::transform(result.begin(), result.end(), std::back_inserter(sat), [atTime](const STDPolytope<double> &region) {
                return region.timeSlice(atTime);
            });
        }
        sat.erase(std::remove_if(sat.begin(), sat.end(), [](const auto &region) { return region.empty(); }), sat.end());

        double probability = 0.0;
        double error = 0.0;

        auto calculator = ProbabilityCalculator();
        probability += calculator.getProbabilityForUnionOfPolytopesUsingMonteCarlo(
            sat,
            this->plt.getDistributionsNormalized(),
            3,
            50000,
            error
        );

        return {probability, error};
    }

    std::vector<STDPolytope<double>> RegionModelChecker::satisfiesHandler(const ParametricLocationTree::Node& node, const Formula &formula, double atTime) {
        switch (formula.getType()) {
            case Formula::Type::ContinuousAtomicProperty: {
                auto result = std::vector<STDPolytope<double>>();
                auto region = this->cfml(node, formula.getContinuousAtomicProperty()->place, formula.getContinuousAtomicProperty()->value);
                if (!region.empty())
                    result.push_back(region);
                return result;
            }
            case Formula::Type::DiscreteAtomicProperty: {
                auto result = std::vector<STDPolytope<double>>();
                auto region = this->dfml(node, formula.getDiscreteAtomicProperty()->place, formula.getDiscreteAtomicProperty()->value);
                if (!region.empty())
                    result.push_back(region);
                return result;
            }
            case Formula::Type::Conjunction: {
                return this->conj(
                    this->satisfiesHandler(node, formula.getConjunction()->left, atTime),
                    this->satisfiesHandler(node, formula.getConjunction()->right, atTime)
                );
            }
            case Formula::Type::Negation: {
                return this->neg(node, this->satisfiesHandler(node, formula.getNegation()->formula, atTime));
            }
            case Formula::Type::Until: {
                return this->until(node, *formula.getUntil(), atTime);
            }
        }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
        throw std::logic_error("unhandled formula type in RegionModelChecker::satisfiesHandler");
#pragma clang diagnostic pop
    }

    //TODO this checks x_p <= u. Should I change this?
    STDPolytope<double> RegionModelChecker::cfml(const ParametricLocationTree::Node& node, const std::string& placeId, int value) {
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
            value
        );

        return regionIntersected;
    }

    STDPolytope<double> RegionModelChecker::dfml(const ParametricLocationTree::Node &node, const std::string& placeId, int value) {
        const auto &discretePlaces = this->hpng->getDiscretePlaces();
        if (discretePlaces.count(placeId) == 0)
            throw std::invalid_argument(std::string("no such discrete place in model: ") + placeId);

        auto placeOffset = std::distance(discretePlaces.begin(), discretePlaces.find(placeId));
        if (node.getParametricLocation().getDiscreteMarking().at(placeOffset) == value) {
            return node.getRegion();
        }
        return STDPolytope<double>::Empty();
    }


    std::vector<STDPolytope<double>> RegionModelChecker::conj(const std::vector<STDPolytope<double>>& a, const std::vector<STDPolytope<double>>& b) {
        std::vector<STDPolytope<double>> sat;
        for(const STDPolytope<double>& ai : a) {
            for (const STDPolytope<double>& bi : b) {
                STDPolytope<double> res = ai.intersect(bi);
                if (!res.empty()) {
                    sat.push_back(res);
                }
            }
        }
        return sat;
    }

    std::vector<STDPolytope<double>> RegionModelChecker::neg(const ParametricLocationTree::Node& node, const std::vector<STDPolytope<double>>& subSat) {
        const STDPolytope<double> &nodeRegion = node.getRegion();

        // If the nested satisfaction set is empty, the whole region satisfies the negation
        if (subSat.empty())
            return {nodeRegion};

        std::vector<STDPolytope<double>> sat;
        for (const auto& polytope : subSat) {
            auto negationResult = nodeRegion.setDifference(polytope);

            // If the satisfaction set is empty, the negations make up the whole satisfaction set.
            if (sat.empty()) {
                std::move(negationResult.begin(), negationResult.end(), std::back_inserter(sat));
                continue;
            }

            // In order to merge the satisfaction set with the new negations we need compute all pairwise intersections
            auto intersections = std::vector<STDPolytope<double>>();
            intersections.reserve(sat.size() * negationResult.size());
            for (const auto& negatedPolytope : negationResult) {
                std::transform(sat.begin(), sat.end(), std::back_inserter(intersections), [&negatedPolytope](STDPolytope<double> satPolytope) {
                    return negatedPolytope.intersect(satPolytope);
                });
            }

            // Get rid off empty polytopes
            intersections.erase(
                std::remove_if(intersections.begin(), intersections.end(), [](STDPolytope<double> p) { return p.empty(); }),
                intersections.end()
            );
            sat = intersections;
        }

        return sat;
    }

    std::vector<STDPolytope<double>> RegionModelChecker::until(const ParametricLocationTree::Node& node, const Until& formula, double atTime) {
        // Construct the upper and lower boundary halfspaces defined by the check-time and the check-time plus until-time.
        hypro::vector_t<double> timeVectorUp = hypro::vector_t<double>::Zero(node.getRegion().dimension());
        timeVectorUp[timeVectorUp.size() - 1] = 1;
        const auto upperLimit = hypro::Halfspace<double>(timeVectorUp, atTime + formula.withinTime);
        const auto lowerLimit = hypro::Halfspace<double>(-timeVectorUp, atTime);

        // Make sure that this node is even reachable within the formula's time frame
        auto fullRegion = node.getRegion();
        fullRegion.insert(upperLimit);
        fullRegion.insert(lowerLimit);
        if (fullRegion.empty())
            return {};

        std::vector<STDPolytope<double>> eventualSat{};
        //region Gather all polytopes that most certainly fulfill `formula`.
        // 1. The polytopes resulting from recursively handled child locations
        for (const auto& childNode : this->plt.getChildNodes(node)) {
            auto childSat = this->until(childNode, formula, atTime);
            std::move(childSat.begin(), childSat.end(), std::back_inserter(eventualSat));
        }
        // 2. The polytopes in the current region that fulfill `formula.goal`
        auto regionSat = this->satisfiesHandler(node, formula.goal, atTime);
        std::move(regionSat.begin(), regionSat.end(), std::back_inserter(eventualSat));
        //endregion Gather all polytopes that most certainly fulfill `formula`.

        // Get the polytopes where the until is "unfulfilled" immediately. The downward extension is needed multiple
        // times, so it is calculated and cached here already.
        auto deadFormula = Formula(std::make_shared<Conjunction>(
            Formula(std::make_shared<Negation>(formula.pre)),
            Formula(std::make_shared<Negation>(formula.goal))
        ));
        auto deadRegions = std::vector<std::pair<STDPolytope<double>, STDPolytope<double>::Polytope>>();
        for (const auto& deadRegion : this->satisfiesHandler(node, deadFormula, atTime))
            deadRegions.emplace_back(deadRegion, deadRegion.extendDownwards());

        auto sat = std::vector<STDPolytope<double>>();
        for (auto goalRegion : eventualSat) {
            goalRegion = STDPolytope<double>(goalRegion.extendDownwards());

            std::vector<STDPolytope<double>> regionsToRemove{};
            regionsToRemove.reserve(deadRegions.size());
            // Every dead region below the goal region must be subtracted from the extended goal region. More specific,
            // the extended dead region needs to be subtracted.
            for (const auto& otherRegion : deadRegions) {
                // Since all satisfaction polytopes are convex and disjoint: if polytope A intersects with the
                // downward-extension of polytope B, then A must be (at least partially) directly below B but nowhere
                // directly above B.
                if (!STDPolytope<double>(goalRegion).intersect(otherRegion.first).empty())
                    regionsToRemove.emplace_back(STDPolytope<double>(otherRegion.second));
            }

            auto goalSat = goalRegion.setDifference(regionsToRemove);
            std::move(goalSat.begin(), goalSat.end(), std::back_inserter(sat));
        }

        return sat;
    }
}
