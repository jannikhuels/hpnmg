#include "RegionModelChecker.h"

#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "datastructures/Region.h"
#include "modelchecker/Conjunction.h"
#include "modelchecker/Negation.h"
#include "modelchecker/DiscreteAtomicProperty.h"
#include "modelchecker/Formula.h"
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
        auto sat = std::vector<Region>();

        const auto timeHyperplane = STDiagram::createHyperplaneForTime(atTime, this->plt.getDimension());
        for (const auto &node : this->plt.getCandidateLocationsForTime(atTime)) {
            const auto &result = this->satisfiesHandler(node, formula, atTime);

            // Add all result polytopes intersected with the check-time hyperplane to sat
            std::transform(result.begin(), result.end(), std::back_inserter(sat), [&timeHyperplane](auto region) {
                auto intersectedRegion = region.hPolytope.intersect(timeHyperplane);
                auto reducedVertices = intersectedRegion.vertices();

                if (reducedVertices.empty())
                    return Region::Empty();

                for (auto &vertex : reducedVertices)
                    vertex.reduceDimension(vertex.dimension() - 1);

                return Region(Region::PolytopeT(reducedVertices));
            });
        }

        sat.erase(std::remove_if(sat.begin(), sat.end(), [](const auto &region) { return region.hPolytope.empty(); }), sat.end());

        double probability = 0.0;
        double error = 0.0;

        auto calculator = ProbabilityCalculator();
        probability += calculator.getProbabilityForUnionOfRegionsUsingMonteCarlo(
            sat,
            this->plt.getDistributionsNormalized(),
            3,
            50000,
            error
        );

        return {probability, error};
    }

    std::vector<Region> RegionModelChecker::satisfiesHandler(const ParametricLocationTree::Node& node, const Formula &formula, double atTime) {
        switch (formula.getType()) {
            case Formula::Type::ContinuousAtomicProperty: {
                auto result = std::vector<Region>();
                auto region = this->cfml(node, formula.getContinuousAtomicProperty()->place, formula.getContinuousAtomicProperty()->value);
                if (!region.hPolytope.empty())
                    result.push_back(region);
                return result;
            }
            case Formula::Type::DiscreteAtomicProperty: {
                auto result = std::vector<Region>();
                auto region = this->dfml(node, formula.getDiscreteAtomicProperty()->place, formula.getDiscreteAtomicProperty()->value);
                if (!region.hPolytope.empty())
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
    Region RegionModelChecker::cfml(const ParametricLocationTree::Node& node, const std::string& placeId, int value)
    {
        const auto &continuousPlaces = this->hpng->getContinuousPlaces();
        auto placeOffset = std::distance(continuousPlaces.begin(), continuousPlaces.find(placeId));

        Region candidateRegion = node.getRegion();

        double drift = node.getParametricLocation().getDrift()[placeOffset];
        Region regionIntersected = STDiagram::intersectRegionForContinuousLevel(
            candidateRegion,
            node.getParametricLocation().getSourceEvent().getGeneralDependenciesNormed(),
            node.getParametricLocation().getContinuousMarkingNormed()[placeOffset],
            drift,
            value
        );

        return regionIntersected;
    }

    Region RegionModelChecker::dfml(const ParametricLocationTree::Node &node, const std::string& placeId, int value) {
        const auto &discretePlaces = this->hpng->getDiscretePlaces();
        auto placeOffset = std::distance(discretePlaces.begin(), discretePlaces.find(placeId));

        if (node.getParametricLocation().getDiscreteMarking().at(placeOffset) == value) {
            return node.getRegion();
        }
        return Region::Empty();
    }


    std::vector<Region> RegionModelChecker::conj(std::vector<Region> a, std::vector<Region> b)
    {
        std::vector<Region> sat;
        for(Region ai : a) {
            for (Region bi : b) {
                Region res = ai.hPolytope.intersect(bi.hPolytope);
                if (!res.hPolytope.empty()) {
                    sat.push_back(res);
                }
            }
        }
        return sat;
    }

    std::vector<Region> RegionModelChecker::neg(const ParametricLocationTree::Node &node, std::vector<Region> a) {
        std::vector<Region> sat;
        Region nodeRegion = node.getRegion();

        if(a.size() == 0) {
            //If sat is empty, return the whole region
            sat.push_back(nodeRegion);
        } else {
            for (Region r : a) {
                for (Halfspace<double> hsp : r.hPolytope.constraints()) {
                    if (nodeRegion.hPolytope.dimension() == hsp.dimension()) {
                        Region b(nodeRegion);
                        b.hPolytope.insert(-hsp);

                        if (!b.hPolytope.empty() &&
                            b.hPolytope.vertices().size() > nodeRegion.hPolytope.dimension()) {
                            sat.push_back(b);
                        }
                    }
                }
            }
        }

        return sat;
    }
}
