#pragma once

#include <vector>
#include "datastructures/HybridPetrinet.h"
#include "ParametricLocationTree.h"

namespace hpnmg {
    class ParseHybridPetrinet {
    private:
        shared_ptr<ParametricLocationTree> parametriclocationTree;
        vector<string> discretePlaceIDs;
        vector<string> continuousPlaceIDs;

    public:
        ParseHybridPetrinet();
        ~ParseHybridPetrinet();

        shared_ptr<ParametricLocationTree> parseHybridPetrinet(shared_ptr<HybridPetrinet> hybridPetrinet, int maxTime);

    };
}
