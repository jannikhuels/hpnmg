#include "ParametricLocation.h"

namespace hpnmg {
    ParametricLocation::ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions) : discreteMarking(numberOfDiscretePlaces), continuousMarking(numberOfContinuousPlaces), drift(numberOfContinuousPlaces), deterministicClock(numberOfDiscretePlaces), generalClock(numberOfGeneralTransitions), generalIntervalBoundLeft(numberOfGeneralTransitions), generalIntervalBoundRight(numberOfGeneralTransitions), conflictProbability(1), dimension(numberOfGeneralTransitions+1) {

    }

    ParametricLocation::ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions, const Event &sourceEvent) : ParametricLocation(numberOfContinuousPlaces, numberOfContinuousPlaces, numberOfGeneralTransitions) {
        setSourceEvent(sourceEvent);
    }

    std::vector<int> ParametricLocation::getDiscreteMarking() const { return discreteMarking; }
    void ParametricLocation::setDiscreteMarking(const std::vector<int> &discreteMarking){ this->discreteMarking = discreteMarking; }

    std::vector<float> ParametricLocation::getContinuousMarking() const{ return continuousMarking; }
    void ParametricLocation::setContinuousMarking(const std::vector<float> &continuousMarking){ this->continuousMarking = continuousMarking; }

    std::vector<float> ParametricLocation::getDrift() const{ return drift; }
    void ParametricLocation::setDrift(const std::vector<float> &continuousMarking){ this->drift = drift; }

    std::vector<int> ParametricLocation::getDeterministicClock() const{ return deterministicClock; }
    void ParametricLocation::setDeterministicClock(const std::vector<int> &deterministicClock){ this->deterministicClock = deterministicClock; }

    std::vector<int> ParametricLocation::getGeneralClock() const{ return generalClock; }
    void ParametricLocation::setGeneralClock(const std::vector<int> &generalClock){ this->generalClock = generalClock; }

    std::vector<int> ParametricLocation::getGeneralIntervalBoundLeft() const{ return generalIntervalBoundLeft; }
    void ParametricLocation::setGeneralIntervalBoundLeft(const std::vector<int> &generalIntervalBoundLeft){ this->generalIntervalBoundLeft = generalIntervalBoundLeft; }

    std::vector<int> ParametricLocation::getGeneralIntervalBoundRight() const{ return generalIntervalBoundRight; }
    void ParametricLocation::setGeneralIntervalBoundRight(const std::vector<int> &generalIntervalBoundRight){ this->generalIntervalBoundRight = generalIntervalBoundRight; }

    Event ParametricLocation::getSourceEvent() const{ return sourceEvent; }
    void ParametricLocation::setSourceEvent(const Event &event){ this->sourceEvent = event; }

    float ParametricLocation::getConflictProbability() const{ return conflictProbability; }
    void ParametricLocation::setConflictProbability(float conflictProbability){ this->conflictProbability = conflictProbability; }

    Region ParametricLocation::getRegion() const{ return region; }
    void ParametricLocation::setRegion(const Region &region){ this->region = region; }
 
    int ParametricLocation::getDimension() const{ return dimension; }
}