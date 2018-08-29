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

    ParametricLocation::ParametricLocation(int numberOfDiscretePlaces, int numberOfContinuousPlaces, int numberOfGeneralTransitions, const Event &sourceEvent) : ParametricLocation(numberOfDiscretePlaces, numberOfContinuousPlaces, numberOfGeneralTransitions) {
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
            generalTransitionsEnabled(parametricLocation.generalTransitionsEnabled),
            sourceEvent(parametricLocation.sourceEvent),
            generalDependenciesNormed(parametricLocation.generalDependenciesNormed),
            integrationIntervals(parametricLocation.integrationIntervals)
    {

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

    vector<double> ParametricLocation::getGeneralDependenciesNormed() {
        //return this->sourceEvent.getGeneralDependenciesNormed();
        return this->generalDependenciesNormed;
    }

    void ParametricLocation::setGeneralDependenciesNormed(const vector<double> &generalDependenciesNormed) {
        //ParametricLocation::generalDependenciesNormed = generalDependenciesNormed;
        //this->sourceEvent.setGeneralDependenciesNormed(generalDependenciesNormed);
        this->generalDependenciesNormed = generalDependenciesNormed;
    }


    const vector<bool> &ParametricLocation::getGeneralTransitionsEnabled() const {
        return generalTransitionsEnabled;
    }

    void ParametricLocation::setGeneralTransitionsEnabled(const vector<bool> &generalTransitionsEnabled) {
        this->generalTransitionsEnabled = generalTransitionsEnabled;
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
                    dependencies[i-1] += value * lowerBounds[i];
                } else {
                    dependencies[i-1] += value * upperBounds[i];
                }
            }
        }

        return time;
    }

    std::vector<double> makeNormed(std::vector<double> in, int dimension) {
        for (int i = in.size(); i < dimension; i++) {
            in.push_back(0);
        }
        return in;
    }

    void ParametricLocation::setIntegrationIntervals(std::vector<std::vector<double>> time, int value) {
        std::vector<std::vector<std::vector<double>>> leftBoundaries = this->getGeneralIntervalBoundLeft();
        std::vector<std::vector<std::vector<double>>> rightBoundaries = this->getGeneralIntervalBoundRight();

        vector<int> generalTransitionsFired = this->getGeneralTransitionsFired();

        std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> result;

        vector<int> counter = vector<int>(this->dimension - 1);
        fill(counter.begin(), counter.end(),0);

        // Collect the Normed Boundaries.

        for (int j = 0; j < generalTransitionsFired.size(); j++) {
            int index = generalTransitionsFired[j];
            result.push_back({index, std::pair<std::vector<double>, std::vector<double>>(makeNormed(leftBoundaries[index][counter[index]], this->dimension), makeNormed(rightBoundaries[index][counter[index]], this->dimension))});
            counter[index] += 1;
        }

        for (int j = 0; j < counter.size(); j++) {
            int firing = counter[j];
            bool enablingTimeGreaterZero = false;
            for (int l = 0; l < this->getGeneralIntervalBoundLeft()[j][firing].size(); l++) {
                if (this->getGeneralIntervalBoundLeft()[j][firing][l] > 0)
                    enablingTimeGreaterZero = true;
            }

            if (this->getGeneralTransitionsEnabled()[j] || enablingTimeGreaterZero) {

                result.push_back({j, std::pair<std::vector<double>, std::vector<double>>(makeNormed(leftBoundaries[j][firing], this->dimension), makeNormed(rightBoundaries[j][firing], this->dimension))});

            }
        }

        // Get the new Boundaries determined by the child events given a specific time
        std::vector<std::pair<std::vector<double>, std::vector<double>>> newBoundaries = Computation::solveEquations(time, value);
        newBoundaries = Computation::replaceValues(newBoundaries);

        // Update the Boundaries
        for (int i = 0; i < newBoundaries.size(); i++) {
            if (newBoundaries[i].first.size()>i && newBoundaries[i].first[i+1] == 0) {
                result[i].second.first = newBoundaries[i].first;
            }
            if (newBoundaries[i].second.size()>i && newBoundaries[i].second[i+1] == 0) {
                result[i].second.second = newBoundaries[i].second;
            }
        }

        // Check if source events influences the bounds
        std::vector<double> startEvent = this->generalDependenciesNormed;
        std::vector<double> t(this->dimension);
        std::fill(t.begin(), t.end(), 0);
        t[0] = value;
        for (int i = 1; i < startEvent.size(); i++) {
            std::vector<double> d = Computation::computeUnequationCut(t, startEvent, i);
            if(d.size() > 0 && d[i] == 0) {
                if (startEvent[i] < 0) {
                    result[i-1].second.first = d;
                }
                if (startEvent[i] > 0) {
                    result[i-1].second.second = d;
                }
            }
        }

        this->integrationIntervals = result;

    }

    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> ParametricLocation::getIntegrationIntervals() const{
        return this->integrationIntervals;
    }

}