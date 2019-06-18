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

        static Halfspace<double> createHalfspaceFromDependenciesNormed(std::vector<double> dependenciesNormed);

        static std::vector<double> makeValidDependencies(std::vector<double> dependencies, int dimension);

        static Halfspace<double> createVerticalHalfspace(std::vector<double> bounds, bool isLeftBound);

        static std::pair<matrix_t<double>,vector_t<double>> createHalfspacesFromTimeInterval(const std::pair<double, double> &interval, int dimension);

        static std::vector<vector_t<double>> hspVectorsFromBounds(std::vector<double> bounds);

        static std::vector<vector_t<double>> directionVectorsFromHspVectors(std::vector<vector_t<double>> hspVectors);

        static vector_t<double> vectorFromBound(double bound, int boundIndex, int dimension);

    public:
        // TODO: Do not create a base region but instead try to read it from the PLT.
        static Region createBaseRegion(int dimension, int maxTime);

        static Region createBaseRegion(int dimension, int maxTime, const std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> &rvIntervals);

        static Region createRegionForVertices(std::vector<Point<double>> vertices);

        static Halfspace<double> createHalfspaceForTime(const double &time, int dimension);

        /**
         * Creates a <code>dimension</code> dimensional hyperplane representing the gien point in time.
         * @param time
         * @param dimension
         * @return
         */
        static Region::PolytopeT createHyperplaneForTime(const double &time, int dimension);

        static Region createRegion(const Region &baseRegion, const Event &sourceEvent, const std::vector<Event> &destinationEvents);

        static void print(const vector<Region> &regionsToPrint, bool cummulative, std::string filename = "out");

        static std::pair<bool, Region> regionIsCandidateForTime(double time,  const Region &region, int dimension);

        static Region createTimeRegion(const Region &region, double time, int dimension);

        static std::pair<bool, Region> regionIsCandidateForTimeInterval(const std::pair<double,double> &interval, const Region &region, int dimension);

        static Region createRegionNoEvent(const Region &baseRegion, const Event &sourceEvent, std::vector<double> leftBounds, std::vector<double> rightBounds);

        /**
         * @TODO Can this be removed in favor of intersectRegionForContinuousLevel()? The linear equation seems to be
         * calculated completely differently.
         */
        static Region legacyIntersectRegionForContinuousLevel(const Region &baseRegion, std::vector<double> continuousDependencies, double drift, double level, bool negate = false);

        static Region intersectRegionForContinuousLevel(const Region &baseRegion, std::vector<double> entryTimeNormed, std::vector<double> markingNormed, double drift, double level);

        static Halfspace<double> createHalfspaceFromEvent(const Event &event, bool isSourceEvent);

        // TODO: Should be isValidEvent but was changed due to automatic creation of PLT
        static Event makeValidEvent(const Event &event, const Region &baseRegion);

        static Intervals createIntervals(const Region &baseInterval);

        static std::vector<Intervals> differenceOfIntervals(std::vector<Intervals> left, std::vector<Intervals> right); 

        static std::vector<Intervals> intersectionOfIntervals(std::vector<Intervals> i1, std::vector<Intervals> i2);

        static std::vector<Region> boundRegionByIntervals(Region r, int maxTime, std::vector<Intervals> intervals, Halfspace<double> timeHsp);

        static std::vector<Intervals> unionOfIntervals(std::vector<Intervals> i1, std::vector<Intervals> i2);

        static std::vector<Intervals> intervalsFromRegions(std::vector<Region> regions);

        static std::vector<Intervals> removeEmptyIntervals(std::vector<Intervals> intervals);

        static std::vector<std::vector<double>> stochasticConstraints(Region region);
    };
}