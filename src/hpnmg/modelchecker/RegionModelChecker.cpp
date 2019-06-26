#include "RegionModelChecker.h"

#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "datastructures/Region.h"
#include "modelchecker/Conjunction.h"
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
        const auto &sat = this->satisfiesHandler(formula, atTime);

        double probability = 0.0;
        double totalError = 0.0;

        auto calculator = ProbabilityCalculator();
        const auto &distributionsNormalized = this->plt.getDistrbutionsNormalized();
        const auto timeHyperplane = STDiagram::createHyperplaneForTime(atTime, this->plt.getDimension());
        for (const auto &region : sat) {
            auto intersectedRegion = region.hPolytope.intersect(timeHyperplane);
            auto reducedVertices = intersectedRegion.vertices();
            for (auto &vertex : reducedVertices) {
                vertex.reduceDimension(vertex.dimension() - 1);
            }

            double error = 0.0;
            probability += calculator.getProbabilityForRegionUsingMonteCarlo(
                Region::PolytopeT(reducedVertices),
                distributionsNormalized,
                3,
                50000,
                error
            );
            totalError += error;
        }

        return {probability, totalError};
    }

    std::vector<Region> RegionModelChecker::satisfiesHandler(const Formula &formula, double atTime) {
        switch (formula.getType()) {
            case Formula::Type::ContinuousAtomicProperty: {
                auto result = std::vector<Region>();
                for (const auto &node : this->plt.getCandidateLocationsForTime(atTime)) {
                    auto region = this->cfml(node, formula.getContinuousAtomicProperty()->place, formula.getContinuousAtomicProperty()->value);
                    if (!region.hPolytope.empty())
                        result.push_back(region);
                }
                return result;
            }
            case Formula::Type::DiscreteAtomicProperty: {
                auto result = std::vector<Region>();
                for (const auto &node : this->plt.getCandidateLocationsForTime(atTime)) {
                    auto region = this->dfml(node, formula.getDiscreteAtomicProperty()->place, formula.getDiscreteAtomicProperty()->value);
                    if (!region.hPolytope.empty())
                        result.push_back(region);
                }
                return result;
            }
            case Formula::Type::Conjunction: {
                return this->conj(
                    this->satisfiesHandler(formula.getConjunction()->left, atTime),
                    this->satisfiesHandler(formula.getConjunction()->right, atTime)
                );
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
}
