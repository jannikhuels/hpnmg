#pragma once

#include "util/Plotter.h"
#include <gmpxx.h>
#include "datastructures/Event.h"
#include "datastructures/Region.h"
#include "exceptions/STDiagramExceptions.h"

using namespace hypro;

namespace hpnmg {
    class STDiagram
    {
    private: 
        
        STDiagram() {};

        static Halfspace<double> createHalfspaceFromDependencies(std::vector<double> dependencies, double offset, bool isVertical);

        static Halfspace<double> createHalfspaceFromEvent(const Event &event, bool isSourceEvent);

        // TODO: Should be isValidEvent but was changed due to automatic creation of PLT
        static Event makeValidEvent(const Event &event, const Region &baseRegion);

        static Halfspace<double> createVerticalHalfspace(std::vector<double> bounds, bool isLeftBound);

        static std::pair<matrix_t<double>,vector_t<double>> createHalfspacesFromTimeInterval(const std::pair<double, double> &interval, int dimension);

        static std::vector<vector_t<double>> hspVectorsFromBounds(std::vector<double> bounds);

        static std::vector<vector_t<double>> directionVectorsFromHspVectors(std::vector<vector_t<double>> hspVectors);

        static vector_t<double> vectorFromBound(double bound, int boundIndex, int dimension);

    public:
        // TODO: Do not create a base region but instead try to read it from the PLT.
        static Region createBaseRegion(int dimension, int maxTime);

        static Region createRegion(const Region &baseRegion, const Event &sourceEvent, const std::vector<Event> &destinationEvents);

        static void print(const vector<Region> &regionsToPrint, bool cummulative);

        static bool regionIsCandidateForTime(double time,  const Region &region, int dimension);

        static Region createTimeRegion(const Region &region, double time, int dimension);

        static bool regionIsCandidateForTimeInterval(const std::pair<double,double> &interval, const Region &region, int dimension);

        static Region createRegionNoEvent(const Region &baseRegion, const Event &sourceEvent, std::vector<double> leftBounds, std::vector<double> rightBounds);

        static Region intersectRegionForContinuousLevel(const Region &baseRegion, std::vector<double> continuousDependencies, double drift, double level);

    };
}