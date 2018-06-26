#include "ParametricLocation.h"

namespace hpnmg {

    ParametricLocation::ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces,
                                           int numberOfGeneralTransitions) : discreteMarking(numberOfDiscretePlaces),
                                                                             continuousMarking(
                                                                                     numberOfContinuousPlaces),
                                                                             drift(numberOfContinuousPlaces),
                                                                             deterministicClock(numberOfDiscretePlaces),
                                                                             generalClock(numberOfGeneralTransitions),
                                                                             generalIntervalBoundLeft(
                                                                                     numberOfGeneralTransitions),
                                                                             generalIntervalBoundRight(
                                                                                     numberOfGeneralTransitions),
                                                                             conflictProbability(1),
                                                                             accumulatedProbability(1),
                                                                             dimension(numberOfGeneralTransitions + 1) {
    }

    ParametricLocation::ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces,
                                           int numberOfGeneralTransitions, const Event &sourceEvent)
            : ParametricLocation(numberOfDiscretePlaces, numberOfContinuousPlaces, numberOfGeneralTransitions) {
        setSourceEvent(sourceEvent);
    }

    ParametricLocation::ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces,
                                           int numberOfGeneralTransitions, const Event &sourceEvent,
                                           std::vector<std::vector<std::vector<double>>> generalIntervalBoundLeft,
                                           std::vector<std::vector<std::vector<double>>> generalIntervalBoundRight)
            : ParametricLocation(
            numberOfDiscretePlaces, numberOfContinuousPlaces, numberOfGeneralTransitions, sourceEvent) {
        setGeneralIntervalBoundLeft(generalIntervalBoundLeft);
        setGeneralIntervalBoundRight(generalIntervalBoundRight);
    }

    ParametricLocation::ParametricLocation(std::vector<int> discreteMarking,
                                           std::vector<std::vector<double>> continuousMarking,
                                           std::vector<double> drift, int numberOfGeneralTransitions,
                                           const Event &sourceEvent,
                                           std::vector<std::vector<std::vector<double>>> intervalBoundLeft,
                                           std::vector<std::vector<std::vector<double>>> intervalBoundRight)
            : ParametricLocation(
            discreteMarking.size(), continuousMarking.size(), numberOfGeneralTransitions, sourceEvent,
            intervalBoundLeft, intervalBoundRight) {
        setDiscreteMarking(discreteMarking);
        setContinuousMarking(continuousMarking);
        setDrift(drift);
    }

    ParametricLocation::ParametricLocation(const ParametricLocation &parametricLocation) :
            discreteMarking(parametricLocation.discreteMarking),
            continuousMarking(parametricLocation.continuousMarking), drift(parametricLocation.drift),
            deterministicClock(parametricLocation.deterministicClock), generalClock(parametricLocation.generalClock),
            generalIntervalBoundLeft(parametricLocation.generalIntervalBoundLeft),
            generalIntervalBoundRight(parametricLocation.generalIntervalBoundRight),
            conflictProbability(parametricLocation.conflictProbability),
            accumulatedProbability(parametricLocation.accumulatedProbability),
            dimension(parametricLocation.dimension),
            generalTransitionFired(parametricLocation.generalTransitionFired),
            sourceEvent(parametricLocation.sourceEvent) {

    }


    std::vector<int> ParametricLocation::getDiscreteMarking() const { return discreteMarking; }

    void ParametricLocation::setDiscreteMarking(
            const std::vector<int> &discreteMarking) { this->discreteMarking = discreteMarking; }

    std::vector<std::vector<double>> ParametricLocation::getContinuousMarking() const { return continuousMarking; }

    void ParametricLocation::setContinuousMarking(
            const std::vector<std::vector<double>> &continuousMarking) { this->continuousMarking = continuousMarking; }

    std::vector<double> ParametricLocation::getDrift() const { return drift; }

    void ParametricLocation::setDrift(const std::vector<double> &drift) { this->drift = drift; }

    std::vector<std::vector<double>> ParametricLocation::getDeterministicClock() const { return deterministicClock; }

    void ParametricLocation::setDeterministicClock(
            const std::vector<std::vector<double>> &deterministicClock) { this->deterministicClock = deterministicClock; }

    std::vector<std::vector<double>> ParametricLocation::getGeneralClock() const { return generalClock; }

    void ParametricLocation::setGeneralClock(
            const std::vector<std::vector<double>> &generalClock) { this->generalClock = generalClock; }

    std::vector<std::vector<std::vector<double>>>
    ParametricLocation::getGeneralIntervalBoundLeft() const { return generalIntervalBoundLeft; }

    void ParametricLocation::setGeneralIntervalBoundLeft(
            const std::vector<std::vector<std::vector<double>>> &generalIntervalBoundLeft) {
        this->generalIntervalBoundLeft = generalIntervalBoundLeft;
    }

    std::vector<std::vector<std::vector<double>>>
    ParametricLocation::getGeneralIntervalBoundRight() const { return generalIntervalBoundRight; }

    void ParametricLocation::setGeneralIntervalBoundRight(
            const std::vector<std::vector<std::vector<double>>> &generalIntervalBoundRight) {
        this->generalIntervalBoundRight = generalIntervalBoundRight;
    }

    Event ParametricLocation::getSourceEvent() const { return sourceEvent; }

    void ParametricLocation::setSourceEvent(const Event &event) { this->sourceEvent = event; }

    double ParametricLocation::getConflictProbability() const { return conflictProbability; }

    void ParametricLocation::setConflictProbability(double conflictProbability) {
        this->conflictProbability = conflictProbability; }

    double ParametricLocation::getAccumulatedProbability() const { return accumulatedProbability; }

    void ParametricLocation::setAccumulatedProbability(double accumulatedProbability) {
        this->accumulatedProbability = accumulatedProbability; }

    int ParametricLocation::getDimension() const { return dimension; }

    std::vector<int> ParametricLocation::getGeneralTransitionsFired() const { return generalTransitionFired; }

    void ParametricLocation::setGeneralTransitionsFired(std::vector<int> generalTransitionsFired) {
        this->generalTransitionFired = generalTransitionsFired; }

    const vector<double> &ParametricLocation::getGeneralDependenciesNormed() const {
        return generalDependenciesNormed;
    }

    void ParametricLocation::setGeneralDependenciesNormed(const vector<double> &generalDependenciesNormed) {
        ParametricLocation::generalDependenciesNormed = generalDependenciesNormed;
        this->sourceEvent.setGeneralDependencies(generalDependenciesNormed);
    }

    const vector<bool> &ParametricLocation::getGeneralTransitionsEnabled() const {
        return generalTransitionsEnabled;
    }

    void ParametricLocation::setGeneralTransitionsEnabled(const vector<bool> &generalTransitionsEnabled) {
        ParametricLocation::generalTransitionsEnabled = generalTransitionsEnabled;
    }

    double ParametricLocation::getEarliestEntryTime() {
        return getMinimumTime(generalIntervalBoundLeft, generalIntervalBoundRight);
    }

    double ParametricLocation::getLatestEntryTime() {
        return getMinimumTime(generalIntervalBoundRight, generalIntervalBoundLeft);
    }

    /*
     * To get the maximum time, swap lower and upper boundaries.
     */
    double ParametricLocation::getMinimumTime(vector<vector<vector<double>>> lowerBoundaries, vector<vector<vector<double>>> upperBoundaries) {
        double time = sourceEvent.getTime();
        vector<double> dependencies = sourceEvent.getGeneralDependencies();
        vector<int> counter = vector<int>(dimension -1);
        fill(counter.begin(), counter.end(),0);
        for (int &firedTransition : generalTransitionFired)
            counter[firedTransition] +=1;


        while (dependencies.size() > 0) {
            double value = dependencies.back();
            dependencies.pop_back();

            int transitionPosition = generalTransitionFired[dependencies.size()];
            int firingTime = counter[transitionPosition];
            vector<double> lowerBounds = lowerBoundaries[transitionPosition][firingTime - 1];
            vector<double> upperBounds = upperBoundaries[transitionPosition][firingTime - 1];

            counter[transitionPosition] -= 1;

            if (value >= 0) {
                time += value * lowerBounds[0];
            } else {
                time += value * upperBounds[0];
            }

            for (int i = 1; i <= dependencies.size(); ++i) {
                if (value >= 0) {
                    dependencies[i] += value * lowerBounds[i];
                } else {
                    dependencies[i] += value * upperBounds[i];
                }
            }
        }

        return time;
    }
}