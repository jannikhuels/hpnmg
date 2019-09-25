#include "ModelChecker.h"

namespace hpnmg {
    bool ModelChecker::sortIntervals(const carl::Interval<double> &first, const carl::Interval<double> &second) {
        return first.lower() < second.lower();
    }

    std::vector<carl::Interval<double>> ModelChecker::insertIntervalBoundariesForRegionAtTime(const Region &region, double time, double dimension, std::vector<carl::Interval<double>> &intervals) {
        Region timeRegion = STDiagram::createTimeRegion(region,time,dimension);
        Box<double> boundingBox = Converter<double>::toBox(timeRegion, CONV_MODE::ALTERNATIVE);
        std::vector<carl::Interval<double>> boundaries = boundingBox.intervals();
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
                if (regionIntersected.vertices().size() > 0 && STDiagram::regionIsCandidateForTime(time,regionIntersected,parametricLocationTree.getDimension()).first == CONTAINMENT::YES) { // TODO: what about the other Containment options?
                    ModelChecker::insertIntervalBoundariesForRegionAtTime(regionIntersected, time, parametricLocationTree.getDimension(), intervals);
                }
            } catch (IllegalIntersectionLevelException e)  {
                
            }
        }
        return intervals;
    }

    std::vector<Region> ModelChecker::discreteFormulaRegionsAtTime(shared_ptr<hpnmg::ParametricLocationTree> parametricLocationTree, int placeIndex, int value, double time, bool negate)
    {
        std::vector<Region> regions;
        std::vector<ParametricLocationTree::Node> candidates = parametricLocationTree->getCandidateLocationsForTime(time);
        for(ParametricLocationTree::Node candidateNode : candidates) {
            if (candidateNode.getParametricLocation().getDiscreteMarking().at(placeIndex) == value) {
                if (!negate) {
                    regions.insert(regions.end(), candidateNode.getRegion());
                }                
            } else if(negate) {
                regions.insert(regions.end(), candidateNode.getRegion());
            }
        }
        return regions;
    }

    std::vector<Region> ModelChecker::continuousFormulaRegionsAtTime(shared_ptr<hpnmg::ParametricLocationTree> parametricLocationTree, int placeIndex, int value, double time, bool negate)
    {
        std::vector<Region> regions;
        std::vector<ParametricLocationTree::Node> candidates = parametricLocationTree->getCandidateLocationsForTime(time);
        for(ParametricLocationTree::Node candidateNode : candidates) {
            try {
                Region candidateRegion = candidateNode.getRegion();            
                double drift = candidateNode.getParametricLocation().getDrift()[placeIndex];
                Region regionIntersected = STDiagram::intersectRegionForContinuousLevel(candidateRegion, candidateNode.getParametricLocation().getContinuousMarking()[placeIndex], drift, value);
                if (regionIntersected.vertices().size() > 0 && STDiagram::regionIsCandidateForTime(time,regionIntersected,parametricLocationTree->getDimension()).first == CONTAINMENT::YES) { // TODO: what about the other Containment options?
                    regions.insert(regions.end(), candidateNode.getRegion());
                }
            } catch (IllegalIntersectionLevelException e)  {
                
            }
        }
        return regions;
    }

    Region ModelChecker::dfml(ParametricLocationTree::Node node, int placeIndex, int value, bool neg)
    {
        if (node.getParametricLocation().getDiscreteMarking().at(placeIndex) == value) {
            if (!neg) {
                return node.getRegion();
            }                
        } else if(neg) {
            return node.getRegion();
        }
        return Region::Empty();
    }

    //TODO this checks x_p <= u. Should I change this?
    Region ModelChecker::cfml(ParametricLocationTree::Node node, int placeIndex, int value, bool neg)
    {
        Region candidateRegion = node.getRegion();            
        double drift = node.getParametricLocation().getDrift()[placeIndex];
        Region regionIntersected = STDiagram::intersectRegionForContinuousLevel(candidateRegion, node.getParametricLocation().getContinuousMarking()[placeIndex], drift, value);
        return regionIntersected;
    }

    std::vector<Region> ModelChecker::conj(std::vector<Region> a, std::vector<Region> b)
    {
        std::vector<Region> sat;
        for(Region ai : a) {
            for (Region bi : b) {
                Region res = ai.intersect(bi);
                if (!res.empty()) {
                    sat.push_back(res);
                }
            }
        }
        return sat;
    }

    Region ModelChecker::until(shared_ptr<hpnmg::ParametricLocationTree> plt, ParametricLocationTree::Node node, double time) 
    {
        return Region::Empty();     
    }

    std::vector<Region> ModelChecker::neg(std::vector<Region> regions, Region nodeRegion) 
    {   
        std::vector<Region> negregions;
        std::vector<std::vector<Halfspace<double>>> hsps;

        for(Region r : regions) 
        {
            std::vector<Halfspace<double>> region_hsps;
            for (Halfspace<double> hsp : r.constraints()) {
                if (nodeRegion.dimension() != hsp.dimension()) {
                    continue;
                }
                region_hsps.push_back(hsp);
            }    
            hsps.push_back(region_hsps);                    
        }

        int dimension = nodeRegion.dimension();
        for(int i = 0; i < hsps.size(); i++) {
            for (Halfspace<double> creatorHsp : hsps[i]) {
                Region b(nodeRegion);                
                b.insert(-creatorHsp);
                for (int j = 0; j < hsps.size(); j++) {
                    if (j == i) {
                        continue;
                    } 
                    for (Halfspace<double> constraint : hsps[j]) {
                        std::pair<CONTAINMENT, Region> satisfies = b.satisfiesHalfspace(-constraint);
                        if (satisfies.first == CONTAINMENT::YES && satisfies.second.vertices().size() > dimension) {
                            // TODO: replaced bool by Containment. Containment allows more than just yes/no, which is why this might not be correct
                            b = satisfies.second;
                        }
                    }
                }
                if (b.vertices().size() > dimension) {
                    negregions.push_back(b);
                }                 
            }                       
        }        
        return negregions;
    }
}