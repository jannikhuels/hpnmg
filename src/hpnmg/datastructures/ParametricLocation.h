#pragma once

#include "Event.h"
#include "STDPolytope.h"
#include "helper/Computation.h"
#include "helper/LinearBoundsTree.h"

namespace hpnmg {


    class ParametricLocation {

    private:
        std::vector<int> discreteMarking;
        std::vector<std::vector<double>> continuousMarking;
        std::vector<std::vector<double>> continuousMarkingNormed;
        std::vector<double> drift;
        std::vector<std::vector<double>> deterministicClock;
        std::vector<std::vector<double>> generalClock;
        std::vector<std::vector<std::vector<double>>> generalIntervalBoundLeft;
        std::vector<std::vector<std::vector<double>>> generalIntervalBoundRight;
        std::vector<std::vector<std::vector<double>>> generalIntervalBoundNormedLeft;
        std::vector<std::vector<std::vector<double>>> generalIntervalBoundNormedRight;
        std::vector<int> generalTransitionFired; // order of general transitions, that already fired
        std::vector<bool> generalTransitionsEnabled;
        std::vector<bool> deterministicTransitionsEnabled;
        std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> integrationIntervals;

        void scheduleIntegrationIntervals(int index, std::vector<double> newBound, std::vector<double> splitBound, double boundValue, double splitValue, int boundIndex, int splitIndex, bool parent);
        void setSplitConstraints(std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> &newIntegrationIntervals, int index, int splitIndex, std::vector<double> splitBound, bool upper);
        bool validBound(int index, int boundIndex, std::vector<double> newBound, bool upper);

    public:
        const vector<bool> &getGeneralTransitionsEnabled() const;

        void setGeneralTransitionsEnabled(const vector<bool> &generalTransitionsEnabled);

        const vector<bool> &getDeterministicTransitionsEnabled() const;

        void setDeterministicTransitionsEnabled(const vector<bool> &deterministicTransitionsEnabled);

    private:
        // enabling status in this loc for all general transitions
        Event sourceEvent;
        double conflictProbability;
        double accumulatedProbability;
        int dimension;
    public:

        ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions);

        ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions,
                           const Event &sourceEvent);

        ParametricLocation(const ParametricLocation &parametricLocation);

        ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions,
                           const Event &sourceEvent, std::vector<std::vector<std::vector<double>>> intervalBoundLeft,
                           std::vector<std::vector<std::vector<double>>> intervalBoundRight);

        ParametricLocation(std::vector<int> discreteMarking, std::vector<std::vector<double>> continuousMarking,
                           std::vector<double> drift, int numberOfGeneralTransitions, const Event &sourceEvent,
                           std::vector<std::vector<std::vector<double>>> intervalBoundLeft,
                           std::vector<std::vector<std::vector<double>>> intervalBoundRight);

        std::vector<int> getDiscreteMarking() const;

        void setDiscreteMarking(const std::vector<int> &discreteMarking);

        std::vector<std::vector<double>> getContinuousMarking() const;

        void setContinuousMarking(const std::vector<std::vector<double>> &continuousMarking);

        std::vector<std::vector<double>> getContinuousMarkingNormed() const;

        void setContinuousMarkingNormed(const std::vector<std::vector<double>> &continuousMarkingNormed);

        std::vector<double> getDrift() const;

        void setDrift(const std::vector<double> &drift);

        std::vector<std::vector<double>> getDeterministicClock() const;

        void setDeterministicClock(const std::vector<std::vector<double>> &deterministicClock);

        std::vector<std::vector<double>> getGeneralClock() const;

        void setGeneralClock(const std::vector<std::vector<double>> &generalClock);

        std::vector<std::vector<std::vector<double>>> getGeneralIntervalBoundLeft() const;

        void setGeneralIntervalBoundLeft(const std::vector<std::vector<std::vector<double>>> &generalIntervalBoundLeft);

        std::vector<std::vector<std::vector<double>>> getGeneralIntervalBoundRight() const;

        void setGeneralIntervalBoundRight(const std::vector<std::vector<std::vector<double>>> &generalIntervalBoundRight);

        std::vector<std::vector<std::vector<double>>> getGeneralIntervalBoundNormedLeft() const;

        void setGeneralIntervalBoundNormedLeft(const std::vector<std::vector<std::vector<double>>> &generalIntervalBoundNormedLeft);

        std::vector<std::vector<std::vector<double>>> getGeneralIntervalBoundNormedRight() const;

        void setGeneralIntervalBoundNormedRight(const std::vector<std::vector<std::vector<double>>> &generalIntervalBoundNormedRight);

        Event getSourceEvent() const;

        void setSourceEvent(const Event &event);

        std::string getSourceEventId() const;

        void setSourceEventId(const std::string &id);

        double getConflictProbability() const;

        void setConflictProbability(double conflictProbability);

        double getAccumulatedProbability() const;

        void setAccumulatedProbability(double accumulatedProbability);

        int getDimension() const;

        std::vector<int> getGeneralTransitionsFired() const;

        void setGeneralTransitionsFired(std::vector<int> generalTransitionsFired);

        // TODO not defined and never used
        int getId() const;

        double getEarliestEntryTime();

        double getLatestEntryTime();

        double getMinimumTime(vector<vector<vector<double>>> lowerBoundaries, vector<vector<vector<double>>> upperBoundaries, double time, vector<double> dependencies);

        std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> getIntegrationIntervals() const;

        void setIntegrationIntervals(std::vector<std::vector<double>> time, std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> bounds, double value, std::vector<int> occurings,
                                     int dimension, int maxTime);

        void overwriteIntegrationIntervals(std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> integrationIntervals);

        std::pair<std::vector<double>, std::vector<double>> compare(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, std::pair<std::vector<double>, std::vector<double>> value, int index);

        /**
         * Each element (int transition, ([double] lower, [double] upper)) of the result vector represents the <code>lower</code>
         * and <code>upper</code> limit of the firing time of the <code>transition</code>.
         *
         * If <code>transition</code> occurs multiple times, this represents multiple firings of that transition in chronological order.
         *
         * The vectors <code>lower</code> and <code>upper</code> represent the coefficients of linear (in)equations
         * depending on other firing times, ordered in local firing order [???]
         *
         * @param occurings
         * @param maxTime
         * @param dim
         * @return
         */
        std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> getRVIntervals(std::vector<int> occurings, int maxTime, int dim);

        /**
         * Similar to getRVIntervals() but:
         * 1. The coefficients of the linear equations are sorted in global firing order [?]
         * 2. A RV interval for every possible firing (globally) is returned.
         * 3. The RV intervals themselves are sorted in global firing order [?]
         *
         * @param occurings
         * @param maxTime
         * @return
         */
        std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> getRVIntervalsNormed(std::vector<int> occurings, int maxTime);
    };
}