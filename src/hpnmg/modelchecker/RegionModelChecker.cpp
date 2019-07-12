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
            std::transform(result.begin(), result.end(), std::back_inserter(sat), [atTime](const STDPolytope &region) {
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

    std::vector<STDPolytope> RegionModelChecker::satisfiesHandler(const ParametricLocationTree::Node& node, const Formula &formula, double atTime) {
        switch (formula.getType()) {
            case Formula::Type::ContinuousAtomicProperty: {
                auto result = std::vector<STDPolytope>();
                auto region = this->cfml(node, formula.getContinuousAtomicProperty()->place, formula.getContinuousAtomicProperty()->value);
                if (!region.empty())
                    result.push_back(region);
                return result;
            }
            case Formula::Type::DiscreteAtomicProperty: {
                auto result = std::vector<STDPolytope>();
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
        }

        return {};
    }

    //TODO this checks x_p <= u. Should I change this?
    STDPolytope RegionModelChecker::cfml(const ParametricLocationTree::Node& node, const std::string& placeId, int value) {
        const auto &continuousPlaces = this->hpng->getContinuousPlaces();
        if (continuousPlaces.count(placeId) == 0)
            throw std::invalid_argument(std::string("no such continuous place in model: ") + placeId);

        auto placeOffset = std::distance(continuousPlaces.begin(), continuousPlaces.find(placeId));

        STDPolytope candidateRegion = node.getRegion();

        double drift = node.getParametricLocation().getDrift()[placeOffset];
        STDPolytope regionIntersected = STDiagram::intersectRegionForContinuousLevel(
            candidateRegion,
            node.getParametricLocation().getSourceEvent().getGeneralDependenciesNormed(),
            node.getParametricLocation().getContinuousMarkingNormed()[placeOffset],
            drift,
            value
        );

        return regionIntersected;
    }

    STDPolytope RegionModelChecker::dfml(const ParametricLocationTree::Node &node, const std::string& placeId, int value) {
        const auto &discretePlaces = this->hpng->getDiscretePlaces();
        if (discretePlaces.count(placeId) == 0)
            throw std::invalid_argument(std::string("no such discrete place in model: ") + placeId);

        auto placeOffset = std::distance(discretePlaces.begin(), discretePlaces.find(placeId));
        if (node.getParametricLocation().getDiscreteMarking().at(placeOffset) == value) {
            return node.getRegion();
        }
        return STDPolytope::Empty();
    }


    std::vector<STDPolytope> RegionModelChecker::conj(const std::vector<STDPolytope>& a, const std::vector<STDPolytope>& b) {
        std::vector<STDPolytope> sat;
        for(const STDPolytope& ai : a) {
            for (const STDPolytope& bi : b) {
                STDPolytope res = ai.intersect(bi);
                if (!res.empty()) {
                    sat.push_back(res);
                }
            }
        }
        return sat;
    }

    std::vector<STDPolytope> RegionModelChecker::neg(const ParametricLocationTree::Node& node, const std::vector<STDPolytope>& subSat) {
        const STDPolytope &nodeRegion = node.getRegion();

        // If the nested satisfaction set is empty, the whole region satisfies the negation
        if (subSat.empty())
            return {nodeRegion};

        std::vector<STDPolytope> sat;
        for (const auto& polytope : subSat) {
            auto negationResult = nodeRegion.setDifference(polytope);

            // If the satisfaction set is empty, the negations make up the whole satisfaction set.
            if (sat.empty()) {
                std::move(negationResult.begin(), negationResult.end(), std::back_inserter(sat));
                continue;
            }

            // In order to merge the satisfaction set with the new negations we need compute all pairwise intersections
            auto intersections = std::vector<STDPolytope>();
            intersections.reserve(sat.size() * negationResult.size());
            for (const auto& negatedPolytope : negationResult) {
                std::transform(sat.begin(), sat.end(), std::back_inserter(intersections), [&negatedPolytope](STDPolytope satPolytope) {
                    return negatedPolytope.intersect(satPolytope);
                });
            }

            // Get rid off empty polytopes
            intersections.erase(
                std::remove_if(intersections.begin(), intersections.end(), [](STDPolytope p) { return p.empty(); }),
                intersections.end()
            );
            sat = intersections;
        }

        return sat;
    }
}
