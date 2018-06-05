#pragma once

#include "Event.h"
#include "Region.h"

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
        Event sourceEvent;
        double conflictProbability;
        double accumulatedProbability;
        int dimension;
        std::vector<double> generalDependenciesNormed;
    public:
        const vector<double> &getGeneralDependenciesNormed() const;

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

        double getMinimumTime(std::vector<std::vector<std::vector<double>>> lowerBoundaries, std::vector<std::vector<std::vector<double>>> upperBoundaries);
    };
}