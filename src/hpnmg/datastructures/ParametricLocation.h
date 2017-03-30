#pragma once

#include "Event.h"
#include "Region.h"

namespace hpnmg {
    class ParametricLocation {

    private:
        std::vector<int> discreteMarking;
        std::vector<float> continuousMarking;
        std::vector<float> drift;
        std::vector<int> deterministicClock;
        std::vector<int> generalClock;
        std::vector<int> generalIntervalBoundLeft;
        std::vector<int> generalIntervalBoundRight;
        Event sourceEvent;
        Region region;
        float conflictProbability;
        int dimension;

    public:
        
        ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions);
        ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions, const Event &sourceEvent);

        std::vector<int> getDiscreteMarking() const;
        void setDiscreteMarking(const std::vector<int> &discreteMarking);

        std::vector<float> getContinuousMarking() const;
        void setContinuousMarking(const std::vector<float> &continuousMarking);

        std::vector<float> getDrift() const;
        void setDrift(const std::vector<float> &continuousMarking);

        std::vector<int> getDeterministicClock() const;
        void setDeterministicClock(const std::vector<int> &deterministicClock);

        std::vector<int> getGeneralClock() const;
        void setGeneralClock(const std::vector<int> &generalClock);

        std::vector<int> getGeneralIntervalBoundLeft() const;
        void setGeneralIntervalBoundLeft(const std::vector<int> &generalIntervalBoundLeft);

        std::vector<int> getGeneralIntervalBoundRight() const;
        void setGeneralIntervalBoundRight(const std::vector<int> &generalIntervalBoundRight);

        Event getSourceEvent() const;
        void setSourceEvent(const Event &event);

        float getConflictProbability() const;
        void setConflictProbability(float conflictProbability);

        Region getRegion() const;
        void setRegion(const Region &region);
 
        int getDimension() const;
    };
}