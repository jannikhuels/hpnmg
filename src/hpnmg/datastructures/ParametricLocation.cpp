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

        std::vector<double> leftBound;
        std::vector<double> rightBound;

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

        if (leftNew > leftOld && leftNew <= rightOld) {
            leftBound = value.first;
        }

        if (rightNew < rightOld && rightNew >= leftOld) {
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

    bool ParametricLocation::validBound(int index, int boundIndex, std::vector<double> newBound, bool upper) {
        std::vector<double> leftBound;
        std::vector<double> rightBound;

        double leftOld = Computation::getTime(this->integrationIntervals[index], this->integrationIntervals[index][boundIndex].second.first, 1);
        double leftNew = leftOld;
        double rightOld = Computation::getTime(this->integrationIntervals[index], this->integrationIntervals[index][boundIndex].second.second, 2);
        double rightNew = rightOld;


        if (newBound.size() > boundIndex && !upper) {
            leftNew = Computation::getTime(this->integrationIntervals[index], newBound, 1);
        } else if (newBound.size() > boundIndex ) {
            rightNew = Computation::getTime(this->integrationIntervals[index], newBound, 2);
        }

        if (!upper && (leftNew > leftOld && leftNew <= rightOld)) {
            return true;
        }

        if (upper && (rightNew < rightOld && rightNew >= leftOld)) {
            return true;
        }

        return false;
    }

    void ParametricLocation::setSplitConstraints(std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> &newIntegrationIntervals, int index, int splitIndex, std::vector<double> splitBound, bool upper) {
        int splitDependencyIndex = Computation::getDependencyIndex(splitBound);
        std::vector<double> constraint;
        double constraintValue;
        if (splitDependencyIndex > 0) {
            if (upper) {
                std::vector<double> oldBound = this->integrationIntervals[index][splitDependencyIndex-1].second.second;
                int depIndex = Computation::getDependencyIndex(splitBound, oldBound);
                constraintValue = splitBound[depIndex] - oldBound[depIndex];
                constraint = Computation::computeUnequationCut(splitBound, oldBound, depIndex);
            } else {
                std::vector<double> oldBound = this->integrationIntervals[index][splitDependencyIndex-1].second.first;
                int depIndex = Computation::getDependencyIndex(splitBound, oldBound);
                constraintValue = oldBound[depIndex] - splitBound[depIndex];
                constraint = Computation::computeUnequationCut(oldBound, splitBound, depIndex);
            }

            int startConstraintIndex = Computation::getDependencyIndex(constraint);
            for (int i = startConstraintIndex; i >= 0;i--) {
                std::vector<double> replaceValue;
                if (constraintValue < 0) {
                    if (Computation::isValidBound(this->integrationIntervals[index], startConstraintIndex, constraint, false).second) {
                        this->integrationIntervals[index][startConstraintIndex].second.first = constraint;
                    }
                    if (Computation::isValidBound(newIntegrationIntervals[index], startConstraintIndex, constraint, true).second) {
                        newIntegrationIntervals[index][startConstraintIndex].second.second = constraint;
                    }
                } else {
                    if (Computation::isValidBound(this->integrationIntervals[index], startConstraintIndex, constraint, true).second) {
                        this->integrationIntervals[index][startConstraintIndex].second.second = constraint;
                    }
                    if (Computation::isValidBound(newIntegrationIntervals[index], startConstraintIndex, constraint, false).second) {
                        newIntegrationIntervals[index][startConstraintIndex].second.first = constraint;
                    }
                }
            }
        }
    }

    void ParametricLocation::scheduleIntegrationIntervals(int index, std::vector<double> newBound, std::vector<double> splitBound, double boundValue, double splitValue, int boundIndex, int splitIndex, bool parent) {
        /*std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> newIntegrationIntervals = this->integrationIntervals;

        if (boundValue > 0) {
            this->integrationIntervals[index][boundIndex].second.second = newBound;

        } else {
            this->integrationIntervals[index][boundIndex].second.first = newBound;
        }

        std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> newInterval = Computation::repairInterval(this->integrationIntervals[index], boundIndex);
        if (newInterval.size() > 0) {
            this->integrationIntervals[index] = newInterval;

            if (splitIndex > 0) {
                if (parent) {
                    if ((boundValue > 0 && splitValue > 0) || (boundValue < 0 && splitValue < 0)) {
                        newIntegrationIntervals[index][splitIndex].second.first = splitBound;
                        this->integrationIntervals[index][splitIndex].second.second = splitBound;
                    } else {
                        this->integrationIntervals[index][splitIndex].second.first = splitBound;
                        newIntegrationIntervals[index][splitIndex].second.second = splitBound;
                    }
                } else {
                    if ((boundValue > 0 && splitValue > 0) || (boundValue < 0 && splitValue < 0)) {
                        newIntegrationIntervals[index][splitIndex].second.second = splitBound;
                        this->integrationIntervals[index][splitIndex].second.first = splitBound;
                    } else {
                        this->integrationIntervals[index][splitIndex].second.second = splitBound;
                        newIntegrationIntervals[index][splitIndex].second.first = splitBound;
                    }
                }

                std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> newSplitOne = Computation.repairInterval(this->integrationIntervals[index], splitIndex);
                if (newSplitOne.size() > 0) {

                }
            }
        } else {
            this->integrationIntervals.erase(this->integrationIntervals.begin()+index);
        }*/

        /*std::pair<bool, bool> isValidBound;
        if (boundValue > 0) {
            isValidBound = Computation::isValidBound(this->integrationIntervals[index], boundIndex, newBound, true);
            if (true) {
                this->integrationIntervals[index][boundIndex].second.second = newBound;
                //this->integrationIntervals[index] = Computation::setBound(this->integrationIntervals[index], boundIndex, newBound, true);
            }

        } else {
            isValidBound = Computation::isValidBound(this->integrationIntervals[index], boundIndex, newBound, false);
            if (true) {
                this->integrationIntervals[index][boundIndex].second.first = newBound;
                //this->integrationIntervals[index] = Computation::setBound(this->integrationIntervals[index], boundIndex, newBound, false);
            }
        }*/



        /*bool validSplitBound = false;
        bool splitBoundUpper;
        if (splitIndex >= 0 && isValidBound.first) {
            validSplitBound = Computation::isValidBound(this->integrationIntervals[index], splitIndex, splitBound, false).first &&  Computation::isValidBound(this->integrationIntervals[index], splitIndex, splitBound, true).first;
            if (validSplitBound) {
                if (parent) {
                    if ((boundValue > 0 && splitValue > 0) || (boundValue < 0 && splitValue < 0)) {
                        newIntegrationIntervals[index][splitIndex].second.first = splitBound;
                        this->integrationIntervals[index][splitIndex].second.second = splitBound;
                        splitBoundUpper = true;
                    } else {
                        this->integrationIntervals[index][splitIndex].second.first = splitBound;
                        newIntegrationIntervals[index][splitIndex].second.second = splitBound;
                        splitBoundUpper = false;
                    }
                } else {
                    if ((boundValue > 0 && splitValue > 0) || (boundValue < 0 && splitValue < 0)) {
                        newIntegrationIntervals[index][splitIndex].second.second = splitBound;
                        this->integrationIntervals[index][splitIndex].second.first = splitBound;
                        splitBoundUpper = false;
                    } else {
                        this->integrationIntervals[index][splitIndex].second.second = splitBound;
                        newIntegrationIntervals[index][splitIndex].second.first = splitBound;
                        splitBoundUpper = true;
                    }
                }
            }
        }*/
        /*bool splitBoundUpper;
        if (splitIndex >= 0) {
            bool validSplitBound =
                    Computation::isValidBound(this->integrationIntervals[index], splitIndex, splitBound,
                                              false).first &&
                    Computation::isValidBound(this->integrationIntervals[index], splitIndex, splitBound,
                                              true).first;
            if (validSplitBound) {
                if (parent) {
                    if ((boundValue > 0 && splitValue > 0) || (boundValue < 0 && splitValue < 0)) {
                        newIntegrationIntervals[index] = Computation::setBound(newIntegrationIntervals[index],
                                                                               splitIndex, splitBound, false);
                        this->integrationIntervals[index] = Computation::setBound(this->integrationIntervals[index],
                                                                                  splitIndex, splitBound, true);
                        splitBoundUpper = true;
                    } else {
                        this->integrationIntervals[index] = Computation::setBound(this->integrationIntervals[index],
                                                                                  splitIndex, splitBound, false);
                        newIntegrationIntervals[index] = Computation::setBound(newIntegrationIntervals[index],
                                                                               splitIndex, splitBound, true);
                        splitBoundUpper = false;
                    }
                } else {
                    if ((boundValue > 0 && splitValue > 0) || (boundValue < 0 && splitValue < 0)) {
                        newIntegrationIntervals[index] = Computation::setBound(newIntegrationIntervals[index],
                                                                               splitIndex, splitBound, true);
                        this->integrationIntervals[index] = Computation::setBound(this->integrationIntervals[index],
                                                                                  splitIndex, splitBound, false);
                        splitBoundUpper = false;
                    } else {
                        this->integrationIntervals[index] = Computation::setBound(this->integrationIntervals[index],
                                                                                  splitIndex, splitBound, true);
                        newIntegrationIntervals[index] = Computation::setBound(newIntegrationIntervals[index],
                                                                               splitIndex, splitBound, false);
                        splitBoundUpper = true;
                    }
                }
                this->setSplitConstraints(newIntegrationIntervals, index, splitIndex, splitBound, splitBoundUpper);
                this->integrationIntervals.push_back(newIntegrationIntervals[index]);
            }
        }*/
    }


    void ParametricLocation::overwriteIntegrationIntervals(std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> integrationIntervals){
        this->integrationIntervals = integrationIntervals;
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

        this->integrationIntervals.push_back(result);

        //std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> integrationIntervals;
        //integerationIntervals.push_back(result);

        /**
         * CheckTime Vector
         */
        std::vector<double> t(dimension);
        std::fill(t.begin(), t.end(), 0);
        t[0] = value;

        /**
         * Check if source events influences the bounds
         */
        std::vector<double> startEvent = this->getSourceEvent().getTimeVector(dimension);
        int k = Computation::getDependencyIndex(startEvent, t);
        if (k > 0) {
            std::vector<double> newBound = Computation::computeUnequationCut(startEvent, t, k);
            bool upper = Computation::isUpper(startEvent, t, k);
            /*this->integrationIntervals = Computation::setBound(this->integrationIntervals[0], k-1, newBound, upper);
            this->integrationIntervals = Computation::repairIntervals(this->integrationIntervals, k-1);*/

            this->integrationIntervals = Computation::setBoundWithSimpleSplit(this->integrationIntervals[0], k-1, newBound, upper);

            for (int intervalIndex=0; intervalIndex<this->integrationIntervals.size(); intervalIndex++) {
                if ((this->integrationIntervals[intervalIndex][0].second.first[0] >= this->integrationIntervals[intervalIndex][0].second.second[0]) || this->integrationIntervals[intervalIndex][0].second.second[0] < 0) {
                    this->integrationIntervals.erase(this->integrationIntervals.begin() + intervalIndex);
                    intervalIndex--;
                    continue;
                }
                if (this->integrationIntervals[intervalIndex][0].second.first[0] < 0) {
                    this->integrationIntervals[intervalIndex][0].second.first[0] = 0;
                }
            }



            /*std::vector<double> valueToReplace;
            double boundValue = startEvent[k] - t[k];
            if(boundValue > 0) {
                valueToReplace = result[k-1].second.second;
            } else {
                valueToReplace = result[k-1].second.first;
            }
            int n = Computation::getDependencyIndex(newBound, valueToReplace);
            std::vector<double> splitBound;
            double splitValue = 0;
            if (n > 0) {
                if (boundValue > 0) {
                    splitValue = newBound[n] - result[n-1].second.second[n];
                    splitBound = Computation::computeUnequationCut(newBound, result[k-1].second.second, n);
                } else {
                    splitValue = result[n-1].second.first[n] - newBound[n];
                    splitBound = Computation::computeUnequationCut(result[k-1].second.first, newBound);
                }
                this->scheduleIntegrationIntervals(0, newBound, splitBound, boundValue, splitValue, k-1, n-1, true);

            }
            else if (n==0) {
                this->scheduleIntegrationIntervals(0, newBound, splitBound, boundValue, 0, k-1, -1, true);
            }*/
        }

        /**
         * Child Events
         */
         if (time.size() > 0) {

             for (vector<double> childEntryTime : time) {
                 std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> copyOfIntegrationIntervals = this->integrationIntervals;
                for (int i = 0; i < copyOfIntegrationIntervals.size(); i++) {
                    int k = Computation::getDependencyIndex(childEntryTime, t);
                    if (k > 0) {
                        std::vector<double> newBound = Computation::computeUnequationCut(t, childEntryTime, k);
                        std::vector<double> valueToReplace;
                        bool upper = Computation::isUpper(t, childEntryTime, k);
                        //std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> locals = Computation::setBound(this->integrationIntervals[i], k-1, newBound, upper);
                        std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> locals = Computation::setBoundWithSimpleSplit(this->integrationIntervals[i], k-1, newBound, upper);
                        //locals = Computation::repairIntervals(locals, k-1);
                        this->integrationIntervals.erase(this->integrationIntervals.begin()+i);
                        for (auto item : locals) {
                            this->integrationIntervals.push_back(item);
                        }

                        /*double boundValue = t[k] - childEntryTime[k];
                        if(boundValue < 0) {
                            valueToReplace = copyOfIntegrationIntervals[i][k-1].second.first;
                        } else {
                            valueToReplace = copyOfIntegrationIntervals[i][k-1].second.second;
                        }
                        int n = Computation::getDependencyIndex(newBound, valueToReplace);
                        std::vector<double> splitBound;
                        double splitValue = 0;
                        if (n > 0) {
                            if (boundValue > 0) {
                                splitValue = newBound[n] - copyOfIntegrationIntervals[i][k-1].second.second[n];
                                splitBound = Computation::computeUnequationCut(newBound, copyOfIntegrationIntervals[i][k-1].second.second, n);
                            } else {
                                splitValue = copyOfIntegrationIntervals[i][k-1].second.first[n] - newBound[n];
                                splitBound = Computation::computeUnequationCut(copyOfIntegrationIntervals[i][k-1].second.first, newBound, n);
                            }
                            this->scheduleIntegrationIntervals(i, newBound, splitBound, boundValue, splitValue, k-1, n-1, false);

                        } else if (n==0) {
                            this->scheduleIntegrationIntervals(i, newBound, splitBound, boundValue, 0, k-1, -1, false);
                        }*/
                    }
                }
                 //this->integrationIntervals = copyOfIntegrationIntervals;
             }
             for (int intervalIndex=0; intervalIndex<this->integrationIntervals.size(); intervalIndex++) {
                 if ((this->integrationIntervals[intervalIndex][0].second.first[0] >= this->integrationIntervals[intervalIndex][0].second.second[0]) || this->integrationIntervals[intervalIndex][0].second.second[0] < 0) {
                     this->integrationIntervals.erase(this->integrationIntervals.begin() + intervalIndex);
                     intervalIndex--;
                     continue;
                 }
                 if (this->integrationIntervals[intervalIndex][0].second.first[0] < 0) {
                     this->integrationIntervals[intervalIndex][0].second.first[0] = 0;
                 }
             }
             //this->integrationIntervals = Computation::repairIntervals(copyOfIntegrationIntervals);
         }

         /**
          * Ensure that the intervals are valid
          */

        /*std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> newIntervals;
          for (int intervalIndex = 0; intervalIndex < this->integrationIntervals.size(); intervalIndex++) {
              int lastIndex = this->integrationIntervals[intervalIndex].size() - 1;
              std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> result = this->integrationIntervals[intervalIndex];
              for (int n = lastIndex; n >= 0; n--) {
                  std::vector<double> lowerBound = result[n].second.first;
                  std::vector<double> upperBound = result[n].second.second;
                  int index = Computation::getDependencyIndex(lowerBound, upperBound);
                  if (index > 0) {
                      std::vector<double> newBound = Computation::computeUnequationCut(lowerBound, upperBound, index);
                      bool upper = (lowerBound[index] - upperBound[index]) > 0 ? true : false;
                      std::pair<bool, bool> validationPair = Computation::isValidBound(this->integrationIntervals[intervalIndex], index-1, newBound, upper);
                      bool isValidBound = validationPair.first && validationPair.second;
                      if (isValidBound) {
                          if (upper) {
                              result[index-1].second.second = newBound;
                          } else {
                              result[index-1].second.first = newBound;
                          }
                      } else if (!validationPair.first) {
                          this->integrationIntervals.erase(this->integrationIntervals.begin() + intervalIndex);
                          break;
                      }
                  } else {
                      if (index == -1) {
                          this->integrationIntervals.erase(this->integrationIntervals.begin() + intervalIndex);
                          break;
                      }
                      if (index == 0) {
                          if (lowerBound[0] < this->integrationIntervals[intervalIndex][n].second.first[0]) {
                              result[n].second.first = this->integrationIntervals[intervalIndex][n].second.first;
                          }
                          if (upperBound[0] > this->integrationIntervals[intervalIndex][n].second.second[0]) {
                              result[n].second.second = this->integrationIntervals[intervalIndex][n].second.second;
                          }
                          if (lowerBound[0] < 0) {
                              result[n].second.first = zero;
                          }
                          if (upperBound[0] < 0) {
                              result[n].second.second = zero;
                          }
                          if (lowerBound[0] > upperBound[0]) {
                              this->integrationIntervals.erase(this->integrationIntervals.begin() + intervalIndex);
                              break;
                          }
                      }
                      this->integrationIntervals[intervalIndex] = result;
                  }
              }
          }*/
    }

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> ParametricLocation::getIntegrationIntervals() const{
        return this->integrationIntervals;
    }

}