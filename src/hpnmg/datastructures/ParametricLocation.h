#pragma once

#include "Event.h"
#include "Region.h"
#include "helper/Computation.h"

namespace hpnmg {
    class ParametricLocation {

    private:
        std::vector<int> discreteMarking;
        std::vector<std::vector<double>> continuousMarking;
        std::vector<double> drift;
        std::vector<std::vector<double>> deterministicClock;
        std::vector<std::vector<double>> generalClock;
        std::vector<std::vector<std::vector<double>>> generalIntervalBoundLeft;
        std::vector<std::vector<std::vector<double>>> generalIntervalBoundRight;
        std::vector<int> generalTransitionFired; // order of general transitions, that already fired
        std::vector<bool> generalTransitionsEnabled;

        std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> integrationIntervals;
        std::vector<double> generalDependenciesNormed;
        void scheduleIntegrationIntervals(int index, std::vector<double> newBound, std::vector<double> splitBound, double boundValue, double splitValue, int boundIndex, int splitIndex, bool parent);
        void setSplitConstraints(std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> &newIntegrationIntervals, int index, int splitIndex, std::vector<double> splitBound, bool upper);
        bool validBound(int index, int boundIndex, std::vector<double> newBound, bool upper);
    public:
        const vector<bool> &getGeneralTransitionsEnabled() const;

        void setGeneralTransitionsEnabled(const vector<bool> &generalTransitionsEnabled);

    private:
        // enabling status in this loc for all general transitions
        Event sourceEvent;
        double conflictProbability;
        double accumulatedProbability;
        int dimension;
    public:
        vector<double> getGeneralDependenciesNormed();

        void setGeneralDependenciesNormed(const vector<double> &generalDependenciesNormed);
        // vector needed for STD is empty while parsing and set when PLT is computed

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

        Event getSourceEvent() const;

        void setSourceEvent(const Event &event);

        double getConflictProbability() const;

        void setConflictProbability(double conflictProbability);

        double getAccumulatedProbability() const;

        void setAccumulatedProbability(double accumulatedProbability);

        int getDimension() const;

        std::vector<int> getGeneralTransitionsFired() const;

        void setGeneralTransitionsFired(std::vector<int> generalTransitionsFired);

        int getId() const;

        double getEarliestEntryTime();

        double getLatestEntryTime();

        double getMinimumTime(vector<vector<vector<double>>> lowerBoundaries, vector<vector<vector<double>>> upperBoundaries, double time, vector<double> dependencies);

        std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> getIntegrationIntervals() const;

        void setIntegrationIntervals(std::vector<std::vector<double>> time, double value, std::vector<int> occurings,
                                     int dimension, int maxTime);

        void overwriteIntegrationIntervals(std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> integrationIntervals);

        std::pair<std::vector<double>, std::vector<double>> compare(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, std::pair<std::vector<double>, std::vector<double>> value, int index);

    };
}