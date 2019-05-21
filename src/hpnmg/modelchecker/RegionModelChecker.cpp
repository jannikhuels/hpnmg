#include "RegionModelChecker.h"

#include <iterator>
#include <memory>

#include "ParseHybridPetrinet.h"

namespace hpnmg {
    RegionModelChecker::RegionModelChecker(HybridPetrinet hpng, double maxTime) :
        hpng(std::make_shared<HybridPetrinet>(hpng)),
        plt(*ParseHybridPetrinet{}.parseHybridPetrinet(this->hpng, maxTime))
    {}

    double RegionModelChecker::satisfies(const Formula &formula, double atTime) {
        switch (formula.getType()) {
            case Formula::Type::DiscreteAtomicProperty:
                for (const auto &node : this->plt.getCandidateLocationsForTime(atTime)) {
                    this->dfml(node, formula.getDiscreteAtomicProperty()->place, formula.getDiscreteAtomicProperty()->value);
                }
        }
        return 0;
    }

    Region RegionModelChecker::dfml(const ParametricLocationTree::Node &node, const std::string& placeId, int value) {
        auto placeOffset = std::distance(
            this->hpng->getDiscretePlaces().begin(),
            this->hpng->getDiscretePlaces().find(placeId)
        );
        if (node.getParametricLocation().getDiscreteMarking().at(placeOffset) == value) {
            return node.getRegion();
        }
        return Region::Empty();
    }
}
