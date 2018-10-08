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
        double time = sourceEvent.getTime();
        vector<double> dependencies = sourceEvent.getGeneralDependencies();
        return getMinimumTime(generalIntervalBoundLeft, generalIntervalBoundRight, time, dependencies);
    }

    double ParametricLocation::getLatestEntryTime() {
        double time = sourceEvent.getTime();
        vector<double> dependencies = sourceEvent.getGeneralDependencies();
        return getMinimumTime(generalIntervalBoundRight, generalIntervalBoundLeft, time, dependencies);
    }

    /*
     * To get the maximum time, swap lower and upper boundaries.
     */
    double ParametricLocation::getMinimumTime(vector<vector<vector<double>>> lowerBoundaries, vector<vector<vector<double>>> upperBoundaries, double time, vector<double> dependencies) {
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

    std::pair<std::vector<double>, std::vector<double>> ParametricLocation::compare(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, std::pair<std::vector<double>, std::vector<double>> value, int index) {

        std::vector<double> leftBound = boundaries[index].second.first;
        std::vector<double> rightBound = boundaries[index].second.second;

        double leftOld = Computation::getTime(boundaries, boundaries[index].second.first, 1);
        double leftNew = leftOld;
        double rightOld = Computation::getTime(boundaries, boundaries[index].second.second, 2);;
        double rightNew = rightOld;


        if (value.first.size() > index) {
            leftNew = Computation::getTime(boundaries, value.first, 1);
        }

        if (value.second.size() > index) {
            rightNew = Computation::getTime(boundaries, value.second, 2);
        }

        if (leftNew > leftOld && leftNew <= rightNew) {
            leftBound = value.first;
        }

        if (rightNew < rightOld && rightNew >= leftNew) {
            rightBound = value.second;
        }

        return {leftBound, rightBound};
    }

    /*std::vector<double> makeNormed(std::vector<double> in, std::vector<int> gtf, int dimension) {
        std::vector<double> result(dimension);
        fill(result.begin(), result.end(), 0);

        for (int i = 0; i < in.size(); i++) {
            result[i] = in[i];
        }

        for (int i = 0; i < gtf.size(); i++) {
            int index = gtf[i] + 1;
            if (in.size() > i+1) {
                result[index] = in[i+1];
                if (index != i+1) {
                    result[i+1] = 0;
                }

            }
        }

        return result;
    }*/

    std::vector<double> fillVector(std::vector<double> in, int dimension) {
        for(int i = in.size(); i < dimension; i++) {
            in.push_back(0);
        }
        return in;
    }

    bool equalVectors(std::vector<double> first, std::vector<double> second) {
        if (first.size() != second.size())
            return false;
        for (int i = 0; i < first.size(); i++) {
            if (first[i] != second[i]) {
                return false;
            }
        }
        return true;
    }

    /*
     * Create the integration intervals for this location, ordered by the firings, i.e. starting with the first firing.
     */
    void ParametricLocation::setIntegrationIntervals(std::vector<std::vector<double>> time, double value,
                                                     std::vector<int> occurings, int dimension, int maxTime) {
        std::vector<std::vector<std::vector<double>>> leftBoundaries = this->getGeneralIntervalBoundLeft();
        std::vector<std::vector<std::vector<double>>> rightBoundaries = this->getGeneralIntervalBoundRight();

        vector<int> generalTransitionsFired = this->getGeneralTransitionsFired();

        /*
         * Initialization of the result vector.
         */
        std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> result;

        vector<int> counter = vector<int>(occurings.size());
        fill(counter.begin(), counter.end(),0);

        vector<double> bound(dimension);
        fill(bound.begin(), bound.end(), 0);
        vector<double> zero = bound;
        vector<double> mTime = bound;
        mTime[0] = maxTime;

        /*
         * First create all intervals for a RV that have already fired.
         */
        for (int realFiring=0; realFiring<generalTransitionsFired.size(); ++realFiring) {
            int transitionId = generalTransitionsFired[realFiring];
            result.push_back({transitionId, { fillVector(leftBoundaries[transitionId][counter[transitionId]], dimension), fillVector(rightBoundaries[transitionId][counter[transitionId]], dimension) } });
            counter[transitionId]++;
        }

        /*
         * Create all intervals for GTs that are currently enabled.
         */
        for (int j = 0; j < occurings.size(); j++) {
            for (int i = counter[j]; i < occurings[j]; i++) {
                int firing = i;
                if (j < this->getGeneralIntervalBoundLeft().size() && firing < this->getGeneralIntervalBoundLeft()[j].size()) {
                    bool enablingTimeGreaterZero = false;
                    for (int l = 0; l < this->getGeneralIntervalBoundLeft()[j][firing].size(); l++) {
                        if (this->getGeneralIntervalBoundLeft()[j][firing][l] > 0)
                            enablingTimeGreaterZero = true;
                    }
                    if (this->getGeneralTransitionsEnabled()[j] || enablingTimeGreaterZero) {
                        result.push_back({j, std::pair<std::vector<double>, std::vector<double>>(
                                fillVector(leftBoundaries[j][firing], dimension),
                                fillVector(rightBoundaries[j][firing], dimension))});
                        continue;
                    }
                }
                result.push_back({j, std::pair<std::vector<double>, std::vector<double>>(zero, mTime)});
            }
        }

        assert(result.size() == dimension-1);

        /*
         * Get the new Boundaries determined by the child events given a specific time and set them accordingly
         */
        std::vector<std::vector<std::pair<std::vector<double>, std::vector<double>>>> candidateBoundaries =
                Computation::solveEquations(time, value);

        for (std::vector<std::pair<std::vector<double>, std::vector<double>>> newBoundaries : candidateBoundaries) {
            // Update the Boundaries
            for (int i = 0; i < newBoundaries.size() && i < result.size(); i++) {
                std::pair<std::vector<double>, std::vector<double>> bound = this->compare(result, newBoundaries[i], i);
                result[i].second.first = bound.first;
                result[i].second.second = bound.second;
            }
        }

        /*
         * Check if source events influences the bounds
         */
        std::vector<double> startEvent = this->getSourceEvent().getTimeVector(dimension);
        std::vector<double> t(dimension);
        std::fill(t.begin(), t.end(), 0);
        t[0] = value;
        for (int i = 1; i < startEvent.size(); i++) {
            std::vector<double> d = Computation::computeUnequationCut(startEvent, t, i);
            if(d.size() > 0 && d[i] == 0) {
                if (startEvent[i] < 0) {
                    std::pair<std::vector<double>, std::vector<double>> bound = this->compare(result, { d, {} }, i-1);
                    result[i-1].second.first = bound.first;
                    result[i-1].second.second = bound.second;
                }
                if (startEvent[i] > 0) {
                    std::pair<std::vector<double>, std::vector<double>> bound = this->compare(result, { {}, d }, i-1);
                    result[i-1].second.first = bound.first;
                    result[i-1].second.second = bound.second;
                }
            }
        }

        for (int i = 0; i < result.size()-1; i++) {
            if(result[i+1].second.first[i+1] != 0) {
                std::vector<double> c = Computation::computeUnequationCut(result[i+1].second.first, result[i+1].second.second, i+1);
                std::pair<std::vector<double>, std::vector<double>> b = this->compare(result, { {}, c }, i);
                result[i].second.second = b.second;
            }
            if(result[i+1].second.second[i+1] != 0) {
                std::vector<double> c = Computation::computeUnequationCut(result[i+1].second.second, result[i+1].second.first, i+1);
                std::pair<std::vector<double>, std::vector<double>> b = this->compare(result, { c, {} }, i);
                result[i].second.first = b.first;
            }
        }

        this->integrationIntervals = result;

    }

    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> ParametricLocation::getIntegrationIntervals() const{
        return this->integrationIntervals;
    }

}