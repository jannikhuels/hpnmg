#pragma once

#include "Event.h"
#include "Region.h"

namespace hpnmg {
    class ParametricLocation {

    private:
        std::vector<int> discreteMarking;
        std::vector<std::vector<double>> continuousMarking;
        std::vector<double> drift;
        std::vector<int> deterministicClock;
        std::vector<int> generalClock;
        std::vector<double> generalIntervalBoundLeft;
        std::vector<double> generalIntervalBoundRight;
        Event sourceEvent;
        double conflictProbability;
        int dimension;

    public:
        
        ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions);
        ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions, const Event &sourceEvent);
        ParametricLocation(const ParametricLocation &parametricLocation);
        ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions, const Event &sourceEvent, std::vector<double> intervalBoundLeft, std::vector<double> intervalBoundRight);
        ParametricLocation(std::vector<int> discreteMarking, std::vector<std::vector<double>> continuousMarking, std::vector<double> drift, int numberOfGeneralTransitions, const Event &sourceEvent, std::vector<double> intervalBoundLeft, std::vector<double> intervalBoundRight);

        std::vector<int> getDiscreteMarking() const;
        void setDiscreteMarking(const std::vector<int> &discreteMarking);

        std::vector<std::vector<double>> getContinuousMarking() const;
        void setContinuousMarking(const std::vector<std::vector<double>> &continuousMarking);

        std::vector<double> getDrift() const;
        void setDrift(const std::vector<double> &continuousMarking);

        std::vector<int> getDeterministicClock() const;
        void setDeterministicClock(const std::vector<int> &deterministicClock);

        std::vector<int> getGeneralClock() const;
        void setGeneralClock(const std::vector<int> &generalClock);

        std::vector<double> getGeneralIntervalBoundLeft() const;
        void setGeneralIntervalBoundLeft(const std::vector<double> &generalIntervalBoundLeft);

        std::vector<double> getGeneralIntervalBoundRight() const;
        void setGeneralIntervalBoundRight(const std::vector<double> &generalIntervalBoundRight);

        Event getSourceEvent() const;
        void setSourceEvent(const Event &event);

        double getConflictProbability() const;
        void setConflictProbability(double conflictProbability);
 
        int getDimension() const;
        int getId() const;
    };
}