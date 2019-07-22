#pragma once

#include <gmpxx.h>
#include <util/plotting/Plotter.h>

#include "datastructures/Event.h"
#include "datastructures/STDPolytope.h"
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
        static STDPolytope<double> createBaseRegion(int dimension, int maxTime);

        static STDPolytope<double> createBaseRegion(int dimension, int maxTime, const std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> &rvIntervals);

        static STDPolytope<double> createRegionForVertices(std::vector<Point<double>> vertices);

        static Halfspace<double> createHalfspaceForTime(const double &time, int dimension);

        /**
         * Creates a <code>dimension</code> dimensional hyperplane representing the gien point in time.
         * @param time
         * @param dimension
         * @return
         */
        static STDPolytope<double>::Polytope createHyperplaneForTime(const double &time, int dimension);

        static STDPolytope<double> createRegion(const STDPolytope<double> &baseRegion, const Event &sourceEvent, const std::vector<Event> &destinationEvents);

        static void print(const vector<STDPolytope<double>> &regionsToPrint, bool cummulative, std::string filename = "out");

        static STDPolytope<double> createRegionNoEvent(const STDPolytope<double> &baseRegion, const Event &sourceEvent, std::vector<double> leftBounds, std::vector<double> rightBounds);

        /**
         * @TODO Can this be removed in favor of intersectRegionForContinuousLevel()? The linear equation seems to be
         * calculated completely differently.
         */
        static STDPolytope<double> legacyIntersectRegionForContinuousLevel(const STDPolytope<double> &baseRegion, std::vector<double> continuousDependencies, double drift, double level, bool negate = false);

        static STDPolytope<double> intersectRegionForContinuousLevel(const STDPolytope<double> &baseRegion, std::vector<double> entryTimeNormed, std::vector<double> markingNormed, double drift, double level);

        static Halfspace<double> createHalfspaceFromEvent(const Event &event, bool isSourceEvent);

        // TODO: Should be isValidEvent but was changed due to automatic creation of PLT
        static Event makeValidEvent(const Event &event, const STDPolytope<double> &baseRegion);

        static std::vector<Intervals> differenceOfIntervals(std::vector<Intervals> left, std::vector<Intervals> right);

        static std::vector<Intervals> intersectionOfIntervals(std::vector<Intervals> i1, std::vector<Intervals> i2);

        static std::vector<STDPolytope<double>> boundRegionByIntervals(STDPolytope<double> r, int maxTime, std::vector<Intervals> intervals, Halfspace<double> timeHsp);

        static std::vector<Intervals> unionOfIntervals(std::vector<Intervals> i1, std::vector<Intervals> i2);

        static std::vector<Intervals> removeEmptyIntervals(std::vector<Intervals> intervals);

    };
}