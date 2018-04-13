#pragma once

#include "ParametricLocationTree.h"

namespace hpnmg {
    class ModelChecker {

    private:
        ModelChecker();

        static std::vector<carl::Interval<double>> insertIntervalBoundariesForRegionAtTime(const Region &region, double time, double dimension, std::vector<carl::Interval<double>> &intervals);

    public:

        static bool sortIntervals(const carl::Interval<double> &first, const carl::Interval<double> &second);

        static std::vector<carl::Interval<double>> discreteFormulaIntervalSetsAtTime(ParametricLocationTree &parametricLocationTree, int placeIndex, int value, double time); 

        static std::vector<carl::Interval<double>> continuousFormulaIntervalSetsAtTime(ParametricLocationTree &parametricLocationTree, int placeIndex, double value, double time);

        static std::vector<Region> discreteFormulaRegionsAtTime(shared_ptr<hpnmg::ParametricLocationTree> parametricLocationTree, int placeIndex, int value, double time);        
    };
}