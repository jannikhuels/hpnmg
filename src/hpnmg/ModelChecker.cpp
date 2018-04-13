#include "ModelChecker.h"

namespace hpnmg {
    bool ModelChecker::sortIntervals(const carl::Interval<double> &first, const carl::Interval<double> &second) {
        return first.lower() < second.lower();
    }

    std::vector<carl::Interval<double>> ModelChecker::insertIntervalBoundariesForRegionAtTime(const Region &region, double time, double dimension, std::vector<carl::Interval<double>> &intervals) {
        Region timeRegion = STDiagram::createTimeRegion(region,time,dimension);
        Box<double> boundingBox = Converter<double>::toBox(timeRegion, CONV_MODE::ALTERNATIVE);
        std::vector<carl::Interval<double>> boundaries = boundingBox.boundaries();
        boundaries.pop_back();
        intervals.insert(intervals.end(), boundaries.begin(), boundaries.end());
        return boundaries;
    }

    std::vector<carl::Interval<double>> ModelChecker::discreteFormulaIntervalSetsAtTime(ParametricLocationTree &parametricLocationTree, int placeIndex, int value, double time) {
        std::vector<carl::Interval<double>> intervals;
        std::vector<ParametricLocationTree::Node> candidates = parametricLocationTree.getCandidateLocationsForTime(time);
        for(ParametricLocationTree::Node candidateNode : candidates) {
            if (candidateNode.getParametricLocation().getDiscreteMarking().at(placeIndex) == value) {
                ModelChecker::insertIntervalBoundariesForRegionAtTime(candidateNode.getRegion(), time, parametricLocationTree.getDimension(), intervals);
            }
        }
        return intervals;
    }

    std::vector<carl::Interval<double>> ModelChecker::continuousFormulaIntervalSetsAtTime(ParametricLocationTree &parametricLocationTree, int placeIndex, double value, double time) {
        std::vector<carl::Interval<double>> intervals;
        std::vector<ParametricLocationTree::Node> candidates = parametricLocationTree.getCandidateLocationsForTime(time);
        for(ParametricLocationTree::Node candidateNode : candidates) {
            Region candidateRegion = candidateNode.getRegion();            
            double drift = candidateNode.getParametricLocation().getDrift()[placeIndex];
            try {
                Region regionIntersected = STDiagram::intersectRegionForContinuousLevel(candidateRegion, candidateNode.getParametricLocation().getContinuousMarking()[placeIndex], drift, value);
                if (regionIntersected.vertices().size() > 0 && STDiagram::regionIsCandidateForTime(time,regionIntersected,parametricLocationTree.getDimension())) {
                    ModelChecker::insertIntervalBoundariesForRegionAtTime(regionIntersected, time, parametricLocationTree.getDimension(), intervals);
                }
            } catch (IllegalIntersectionLevelException e)  {
                
            }
        }
        return intervals;
    }

    std::vector<Region> ModelChecker::discreteFormulaRegionsAtTime(shared_ptr<hpnmg::ParametricLocationTree> parametricLocationTree, int placeIndex, int value, double time)
    {
        std::vector<Region> regions;
        std::vector<ParametricLocationTree::Node> candidates = parametricLocationTree->getCandidateLocationsForTime(time);
        for(ParametricLocationTree::Node candidateNode : candidates) {
            if (candidateNode.getParametricLocation().getDiscreteMarking().at(placeIndex) == value) {
                regions.insert(regions.end(), candidateNode.getRegion());
            }
        }
        return regions;
    }
}