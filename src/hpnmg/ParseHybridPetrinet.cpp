#include "ParseHybridPetrinet.h"

using namespace std;
namespace hpnmg {
    ParseHybridPetrinet::ParseHybridPetrinet() {}

    ParseHybridPetrinet::~ParseHybridPetrinet() {}

    shared_ptr<ParametricLocationTree> ParseHybridPetrinet::parseHybridPetrinet(shared_ptr<HybridPetrinet> hybridPetrinet, int maxTime) {
        // TODO: all floats to double?
        // Add place IDs from map to vector, so the places have an order
        map<string, shared_ptr<DiscretePlace>> discretePlaces = hybridPetrinet->getDiscretePlaces();
        for (auto i = discretePlaces.begin(); i != discretePlaces.end(); ++i) {
            discretePlaceIDs.push_back(i->first);
        }
        map<string, shared_ptr<ContinuousPlace>> continuousPlaces = hybridPetrinet->getContinuousPlaces();
        for (auto i = continuousPlaces.begin(); i != continuousPlaces.end(); ++i) {
            continuousPlaceIDs.push_back(i->first);
        }

        //// Generate root parametric location
        // Get root discrete markings
        vector<int> rootDiscreteMarking;
        for (string &placeId: discretePlaceIDs)
            rootDiscreteMarking.push_back(static_cast<int>(discretePlaces[placeId]->getMarking()));

        // Get root continuous marking
        vector<vector<double>> rootContinuousMarking; // todo: warum eine Matrix?
        for (string &placeId: continuousPlaceIDs) {
            rootContinuousMarking.push_back({continuousPlaces[placeId]->getLevel()});
        }

        vector<double> drift = {2}; // todo: was ist das?

        // Fill left and right bounds
        unsigned long numGeneralTransitions = hybridPetrinet->num_general_transitions();
        vector<double> leftBoundaries(numGeneralTransitions);
        fill(leftBoundaries.begin(), leftBoundaries.end(), 0);
        vector<double> rightBoundaries(numGeneralTransitions);
        fill(rightBoundaries.begin(), rightBoundaries.end(), maxTime);

        ParametricLocation rootLocation = ParametricLocation(rootDiscreteMarking, rootContinuousMarking, drift,
                static_cast<int>(numGeneralTransitions), Event(EventType::Root,vector<double>{0, 0}, 0),
                leftBoundaries, rightBoundaries);

        parametriclocationTree = make_shared<ParametricLocationTree>(rootLocation, maxTime);

        return parametriclocationTree;
    }
}