#include <assert.h>
#include "Computation.h"

namespace hpnmg {

    std::vector<double> Computation::computeUnequationCut(std::vector<double> time1, std::vector<double> time2) {
        unsigned long minSize = std::min(time1.size(), time2.size());
        long k = -1;
        double diffK = 0.0;
        for (long i = minSize - 1; i >= 0; --i) {
            if (time1[i] != time2[i]) {
                k = i;
                diffK = time2[i] - time1[i];
                break;
            }
        }
        std::vector<double> unequationCut(minSize);

        if (k < 1 || diffK == 0.0) {
            std::fill(unequationCut.begin(), unequationCut.end(), 0);
            return unequationCut;
        }

        for (int i = 0; i < minSize; ++i) {
            if (i < k) {
                unequationCut[i] = (time1[i] - time2[i]) / diffK;
            } else {
                unequationCut[i] = 0;
            }
        }


        return unequationCut;
    }

    std::vector<double> Computation::computeUnequationCut(std::vector<double> time1, std::vector<double> time2, int index) {
        // Swap the variables
        double v1 = time1[index];
        time1[index] = time1[time1.size()-1];
        time1[time1.size()-1] = v1;

        double v2 = time2[index];
        time2[index] = time2[time2.size()-1];
        time2[time2.size()-1] = v2;

        std::vector<double> res = computeUnequationCut(time1, time2);

        //Swap back
        double v = res[index];
        res[index] = 0;
        res[res.size()-1] = v;

        return res;
    }

    std::vector<std::vector<std::pair<std::vector<double>, std::vector<double>>>> Computation::solveEquations
            (std::vector<std::vector<double>> time, double value) {

        if (time.size() == 0) {
            std::vector<std::vector<std::pair<std::vector<double>, std::vector<double>>>> results(time.size());
            return results;
        }

        //std::vector<std::pair<std::vector<double>, std::vector<double>>> result(time[0].size()-1);
        std::vector<std::vector<std::pair<std::vector<double>, std::vector<double>>>> results(time.size(),
                                                                                              std::vector<std::pair<std::vector<double>, std::vector<double>>>(time[0].size()-1));



        int size = time[0].size();

        // Create time equation
        std::vector<double> t(size);
        std::fill(t.begin(), t.end(), 0);
        t[0] = value;

        for (int i = size - 1; i > 0; i--) {
            for (int j = 0; j< time.size(); j++){
                auto v = time[j];
                //std::pair<std::vector<double>, std::vector<double>> p({},{});
                if (v[i] > 0) {
                    // lower bound
                    results[j][i-1].first = Computation::computeUnequationCut(t, v, i);
                    //p.first = Computation::computeUnequationCut(t, v, i);

                } else if (v[i] < 0) {
                    // upper bound
                    //p.second = Computation::computeUnequationCut(t,v,i);
                    results[j][i-1].second = Computation::computeUnequationCut(t, v, i);
                }
            }
        }
        return results;
    }


    int solved(std::vector<double> func, int pos) {
        int size = func.size();
        for (int i = size - 1; i >= pos && i > 0; i--) {
            if (func[i] != 0) {
                return i;
            }
        }
        return 0;
    }

    std::vector<double> getSolve(std::vector<double> toSolve, std::pair<std::vector<double>, std::vector<double>> res, int s, int i) {
        std::vector<double> res1 = Computation::replace(toSolve, res.first, s);
        std::vector<double> res2 = Computation::replace(toSolve, res.second, s);

        int s1 = solved(res1, i);
        int s2 = solved(res2, i);

        if (s1 == 0 && res.first.size() > 0) {
            return res1;
        }

        if (s2 == 0 && res.second.size() > 0) {
            return res2;
        }

        if (s1 == i && res.first.size() > 0) {
            return res1;
        }

        if (s2 == i && res.second.size() > 0) {
            return res2;
        }

        return toSolve;
    }

    std::vector<std::pair<std::vector<double>, std::vector<double>>> Computation::replaceValues(std::vector<std::pair<std::vector<double>, std::vector<double>>> in) {
        bool allSolved = true;
        int size = 0;
        if(in.size() == 0 ) {
            return {};
        }
        size = in.size();

        for (int i = 0; i <  size; i++) {
            int s1 = solved(in[i].first, i+1);
            int s2 = solved(in[i].second, i+1);

            if (s1 != 0 && s1 != i+1) {
                allSolved = false;
                in[i].first = getSolve(in[i].first, in[s1-1], s1, i+1);
            }

            if (s2 != 0 && s2 != i+1) {
                allSolved = false;
                in[i].second = getSolve(in[i].second, in[s2-1], s2, i+1);
            }
        }

        if (allSolved)
            return in;
        return Computation::replaceValues(in);
    }

    std::vector<double> Computation::replace(std::vector<double> func, std::vector<double> repl, int index) {
        if (repl.size() != func.size()) {
            return func;
        }
        int factor = func[index];
        func[index] = 0;
        for (int i = 0; i < func.size(); i++) {
            func[i] = func[i] + factor * repl[i];
        }
        return func;
    }

    bool Computation::isSmaller(std::vector<double> f1, std::vector<double> f2, int time) {
        std::vector<double> t(f1.size());
        std::fill(t.begin(), t.end(), 0);
        t[0] = time;

        std::vector<double> t_f1 = Computation::computeUnequationCut(t, f1);
        std::vector<double> t_f2 = Computation::computeUnequationCut(t, f2);

        f1 = t_f1[0] != 0 ? t_f1 : f1;
        f2 = t_f2[0] != 0 ? t_f2 : f2;

        // TODO This implementation is incomplete, needs to be revisited
        //std::vector<double> result = Computation::computeUnequationCut(f1, f2);

        return f1[0] < f2[0];
    }

    bool Computation::isGreater(std::vector<double> f1, std::vector<double> f2, int time) {
        std::vector<double> t(f1.size());
        std::fill(t.begin(), t.end(), 0);
        t[0] = time;

        std::vector<double> t_f1 = Computation::computeUnequationCut(t, f1);
        std::vector<double> t_f2 = Computation::computeUnequationCut(t, f2);

        f1 = t_f1[0] != 0 ? t_f1 : f1;
        f2 = t_f2[0] != 0 ? t_f2 : f2;

        // TODO This implementation is incomplete, needs to be revisited
        //std::vector<double> result = Computation::computeUnequationCut(f1, f2);

        return f1[0] > f2[0];
    }

    double Computation::getTime(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, std::vector<double> dependencies, int mode) {
        while (dependencies.size() > 1) {
            double value = dependencies.back();
            dependencies.pop_back();
            int index = dependencies.size() - 1;

            std::vector<double> lowerBounds;
            std::vector<double> upperBounds;
            if (mode == 1) {
                lowerBounds = boundaries[index].second.first;
                upperBounds = boundaries[index].second.second;
            } else {
                lowerBounds = boundaries[index].second.second;
                upperBounds = boundaries[index].second.first;
            }

            for (int i = 0; i <= dependencies.size()-1; ++i) {
                if (value >= 0) {
                    dependencies[i] += value * lowerBounds[i];
                } else {
                    dependencies[i] += value * upperBounds[i];
                }
            }
        }
        return dependencies[0];
    }

    int Computation::getDependencyIndex(std::vector<double> in1, std::vector<double> in2) {
        assert(in1.size() == in2.size());
        for (int i = in1.size() - 1; i >= 0; i--) {
            if ((in1[i] != 0 || in2[i] != 0) && (in1[i] != in2[i])) {
                return i;
            }
        }
        return -1;
    }

    int Computation::getDependencyIndex(std::vector<double> in) {
        for (int i = in.size() - 1; i >= 0; i--) {
            if (in[i] != 0) {
                return i;
            }
        }
        return -1;
    }

    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> Computation::repairInterval(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> interval, int maxIndex)
    {
        if (maxIndex == -1) {
            maxIndex = interval.size() - 1;
        }
        for(int boundIndex=maxIndex; boundIndex >= 0; boundIndex--) {
            /*if ((interval[0].second.first[0] >= interval[0].second.second[0]) || interval[0].second.second[0] < 0) {
                return {};
            }*/
            if (interval.size() == 0) {
                return interval;
            }
            if (interval[0].second.first[0] < 0) {
                interval[0].second.first[0] = 0;
            }

            std::vector<double> lowerBound = interval[boundIndex].second.first;
            std::vector<double> upperBound = interval[boundIndex].second.second;
            int dependencyIndex = Computation::getDependencyIndex(lowerBound, upperBound);

            if (dependencyIndex > 0) {
                std::vector<double> newBound = Computation::computeUnequationCut(lowerBound, upperBound, dependencyIndex);
                bool upper = Computation::isUpper(lowerBound, upperBound, dependencyIndex);
                int repairBoundIndex = dependencyIndex-1;

                if (upper) {
                    interval[repairBoundIndex].second.second = newBound;
                } else {
                    interval[repairBoundIndex].second.first = newBound;
                }

                if (repairBoundIndex > 0) {
                    int depIndexLower = Computation::getDependencyIndex(interval[repairBoundIndex-1].second.first, newBound);
                    if (depIndexLower > 0) {
                        upper = Computation::isUpper(interval[repairBoundIndex-1].second.first, newBound, depIndexLower);
                        newBound = Computation::computeUnequationCut(interval[repairBoundIndex-1].second.first, newBound, depIndexLower);
                        interval = Computation::setBoundNoSplit(interval, depIndexLower-1, newBound, upper);
                    }

                    if (interval.size() > 0) {
                        int depIndexUpper = Computation::getDependencyIndex(newBound, interval[repairBoundIndex-1].second.second);
                        if (depIndexUpper > 0) {
                            upper = Computation::isUpper(newBound, interval[repairBoundIndex-1].second.second, depIndexUpper);
                            newBound = Computation::computeUnequationCut(newBound, interval[repairBoundIndex-1].second.second, depIndexUpper);
                            interval = Computation::setBoundNoSplit(interval, depIndexUpper-1, newBound, upper);
                        }
                    }
                }
            }
            else {
                if (boundIndex > 0) {
                    //return {};
                }
            }
        }
        return interval;
    }

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> Computation::repairIntervals(std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> intervals, int maxIndex) {
        for (int intervalIndex=0; intervalIndex<intervals.size(); intervalIndex++) {
            std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> i = Computation::repairInterval(intervals[intervalIndex], maxIndex);
            if (i.size() > 0) {
                intervals[intervalIndex] = i;
            } else {
                intervals.erase(intervals.begin()+intervalIndex);
                intervalIndex--;
                continue;
            }
        }
        for (int intervalIndex=0; intervalIndex<intervals.size(); intervalIndex++) {
            if ((intervals[intervalIndex][0].second.first[0] >= intervals[intervalIndex][0].second.second[0]) || intervals[intervalIndex][0].second.second[0] < 0) {
                intervals.erase(intervals.begin() + intervalIndex);
                intervalIndex--;
                continue;
            }
            if (intervals[intervalIndex][0].second.first[0] < 0) {
                intervals[intervalIndex][0].second.first[0] = 0;
            }
        }
        return intervals;
    }

    std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> Computation::setBoundNoSplit(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper) {
        while (boundIndex >= 0) {
            std::vector<double> formerBound;
            if (upper) {
                formerBound = boundaries[boundIndex].second.second;
                if (boundIndex == 0) {
                    if (newBound[0] > formerBound[0]) {
                        boundIndex = -1;
                        return {};
                    }
                }
                boundaries[boundIndex].second.second = newBound;
            } else {
                formerBound = boundaries[boundIndex].second.first;
                if (boundIndex == 0) {
                    if (newBound[0] < formerBound[0]) {
                        boundIndex = -1;
                        return {};
                    }
                }
                boundaries[boundIndex].second.first = newBound;
            }

            int dependencyIndex = Computation::getDependencyIndex(formerBound, newBound);

            if (dependencyIndex > 0) {
                if (upper) {
                    upper = Computation::isUpper(newBound, formerBound, dependencyIndex);
                    newBound = Computation::computeUnequationCut(newBound, formerBound, dependencyIndex);

                } else {
                    upper = Computation::isUpper(formerBound, newBound, dependencyIndex);
                    newBound = Computation::computeUnequationCut(formerBound, newBound, dependencyIndex);
                }
                boundIndex = dependencyIndex-1;
            } else {
                boundIndex = -1;
            }
        }
        return boundaries;
    }

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> Computation::setBound(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper) {
        std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> newBoundaries = boundaries;

        std::vector<double> valueToReplace;
        if(upper) {
            valueToReplace = boundaries[boundIndex].second.second;
            boundaries[boundIndex].second.second = newBound;
        } else {
            valueToReplace = boundaries[boundIndex].second.first;
            boundaries[boundIndex].second.first = newBound;
        }
        int n = Computation::getDependencyIndex(newBound, valueToReplace);
        if (n > 0) {
            if (upper) {
                upper = Computation::isUpper(newBound, valueToReplace, n);
                newBound = Computation::computeUnequationCut(newBound, valueToReplace, n);
            } else {
                upper = Computation::isUpper(valueToReplace, newBound, n);
                newBound = Computation::computeUnequationCut(valueToReplace, newBound);
            }
            boundIndex = n -1;
        } else {
            return {boundaries};
        }

        int dependencyIndex = 1;

        while (dependencyIndex >= 0) {
            std::vector<double> formerBound;
            if (upper) {
                formerBound = boundaries[boundIndex].second.second;
            } else {
                formerBound = boundaries[boundIndex].second.first;
            }

            if (upper) {
                boundaries[boundIndex].second.second = newBound;
                newBoundaries[boundIndex].second.first = newBound;
            } else {
                boundaries[boundIndex].second.first = newBound;
                newBoundaries[boundIndex].second.second = newBound;
            }

            dependencyIndex = Computation::getDependencyIndex(formerBound, newBound);

            if (dependencyIndex > 0) {
                if (upper) {
                    upper = Computation::isUpper(newBound, formerBound, dependencyIndex);
                    newBound = Computation::computeUnequationCut(newBound, formerBound, dependencyIndex);
                } else {
                    upper = Computation::isUpper(formerBound, newBound, dependencyIndex);
                    newBound = Computation::computeUnequationCut(formerBound, newBound, dependencyIndex);
                }
                boundIndex = dependencyIndex-1;
            } else {
                dependencyIndex = -1;
            }
        }

        return {boundaries, newBoundaries};
    }

    std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> Computation::setBoundWithSimpleSplit(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper) {
        std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> newBoundaries = boundaries;



        std::vector<double> valueToReplace;
        if(upper) {
            valueToReplace = boundaries[boundIndex].second.second;
            boundaries[boundIndex].second.second = newBound;
        } else {
            valueToReplace = boundaries[boundIndex].second.first;
            boundaries[boundIndex].second.first = newBound;
        }

        if (boundIndex == 0) {
            if (upper && newBound[0] > valueToReplace[0]) {
                return {newBoundaries};
            }
            if (!upper && newBound[0] < valueToReplace[0]) {
                return {newBoundaries};
            }
        }

        boundaries = Computation::repairInterval(boundaries, boundIndex);



        int n = Computation::getDependencyIndex(newBound, valueToReplace);
        if (n > 0) {
            if (upper) {
                upper = Computation::isUpper(newBound, valueToReplace, n);
                newBound = Computation::computeUnequationCut(newBound, valueToReplace, n);
            } else {
                upper = Computation::isUpper(valueToReplace, newBound, n);
                newBound = Computation::computeUnequationCut(valueToReplace, newBound);
            }
            boundIndex = n -1;
        } else {
            if (boundaries.size() == 0) {
                return {};
            }
            return {boundaries};
        }

        std::vector<double> formerBound;
        if (upper) {
            formerBound = newBoundaries[boundIndex].second.second;
        } else {
            formerBound = newBoundaries[boundIndex].second.first;
        }

        if (upper) {
            if (boundaries.size() > 0) {
                boundaries[boundIndex].second.second = newBound;
            }
            newBoundaries[boundIndex].second.first = newBound;
        } else {
            if (boundaries.size() > 0) {
                boundaries[boundIndex].second.first = newBound;
            }
            newBoundaries[boundIndex].second.second = newBound;
        }

        if (boundaries.size() > 0) {
            boundaries = Computation::repairInterval(boundaries, boundIndex);
        }
        newBoundaries = Computation::repairInterval(newBoundaries, boundIndex);

        if (boundaries.size() > 0 && newBoundaries.size() > 0) {
            return { boundaries, newBoundaries };
        } else if (boundaries.size() > 0) {
            return { boundaries };
        } else if (newBoundaries.size() > 0) {
            return { newBoundaries };
        } else {
            return {};
        }

    }

    std::pair<bool, bool> Computation::isValidBound(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper)
    {
        double epsilon = 0.0001;
        std::vector<double> lowerBound = boundaries[boundIndex].second.first;
        int lowerDependencyIndex = Computation::getDependencyIndex(lowerBound, newBound);

        std::pair<bool, bool> plausibleLeftBound;
        if (lowerDependencyIndex <= 0) {
            if (lowerDependencyIndex == -1) {
                plausibleLeftBound = { true, true };
            } else {
                plausibleLeftBound = { lowerBound[0] <= (newBound[0] + epsilon), lowerBound[0] <= (newBound[0] + epsilon) };
            }

        } else {
            std::vector<double> newBoundToTest = Computation::computeUnequationCut(lowerBound, newBound, lowerDependencyIndex);
            bool setUpper = true;
            if ((lowerBound[lowerDependencyIndex] - newBound[lowerDependencyIndex]) < 0) {
                setUpper = false;
            }
            plausibleLeftBound = Computation::isValidBound(boundaries, lowerDependencyIndex-1, newBoundToTest, setUpper);
        }


        std::vector<double> upperBound = boundaries[boundIndex].second.second;
        int upperDependencyIndex = Computation::getDependencyIndex(upperBound, newBound);

        std::pair<bool, bool> plausibleRightBound;
        if (upperDependencyIndex <= 0) {
            if (upperDependencyIndex == -1) {
                plausibleRightBound = { true, true };
            } else {
                plausibleRightBound = { (newBound[0] - epsilon) <= upperBound[0], (newBound[0] - epsilon) <= upperBound[0] };
            }
        } else {
            std::vector<double> newBoundToTest = Computation::computeUnequationCut(newBound, upperBound, upperDependencyIndex);
            bool setUpper = true;
            if ((newBound[upperDependencyIndex] - upperBound[upperDependencyIndex]) < 0) {
                setUpper = false;
            }
            plausibleRightBound = Computation::isValidBound(boundaries, upperDependencyIndex - 1, newBoundToTest, setUpper);
        }

        if (lowerDependencyIndex <= 0 && upperDependencyIndex <= 0) {
            if (plausibleLeftBound.first && plausibleRightBound.first) {
                return {true, true};
            } else if (!plausibleLeftBound.first && plausibleRightBound.first && !upper) {
                return {true, false};
            } else if (plausibleLeftBound.first && !plausibleRightBound.first && upper) {
                return {true, false};
            }
        }
        return { plausibleLeftBound.first && plausibleRightBound.first,  plausibleLeftBound.second || plausibleRightBound.second};
    }

    void setBound(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> &recursionBound, int boundIndex, std::vector<double> newBound, bool upper) {
        if (Computation::isValidBound(recursionBound, boundIndex, newBound).first) {
            if (upper) {
                recursionBound[boundIndex].second.second = newBound;
            } else {
                recursionBound[boundIndex].second.first = newBound;
            }
        }
    }

    bool Computation::createValidIntervals(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper, std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> &recursionBound)
    {
        std::vector<double> lowerBound = boundaries[boundIndex].second.first;
        int lowerDependencyIndex = Computation::getDependencyIndex(lowerBound, newBound);

        bool plausibleLeftBound;
        if (lowerDependencyIndex <= 0) {
            if (lowerDependencyIndex == -1) {
                plausibleLeftBound = true;
            } else {
                plausibleLeftBound = lowerBound[0] <= newBound[0];
            }

        }

        std::vector<double> upperBound = boundaries[boundIndex].second.second;
        int upperDependencyIndex = Computation::getDependencyIndex(upperBound, newBound);

        bool plausibleRightBound;
        if (upperDependencyIndex <= 0) {
            if (upperDependencyIndex == -1) {
                plausibleRightBound = true;
            } else {
                plausibleRightBound = newBound[0] <= upperBound[0];
            }
        }


        if (lowerDependencyIndex > 0) {
            std::vector<double> newBoundToTest = Computation::computeUnequationCut(lowerBound, newBound, lowerDependencyIndex);
            bool setUpper = true;
            if ((lowerBound[lowerDependencyIndex] - newBound[lowerDependencyIndex]) < 0) {
                setUpper = false;
            }
            setBound(recursionBound, boundIndex, newBound, upper);
            plausibleLeftBound = Computation::createValidIntervals(boundaries, lowerDependencyIndex-1, newBoundToTest, setUpper, recursionBound);
        }

        if (upperDependencyIndex > 0) {
            std::vector<double> newBoundToTest = Computation::computeUnequationCut(newBound, upperBound, upperDependencyIndex);
            bool setUpper = true;
            if ((newBound[upperDependencyIndex] - upperBound[upperDependencyIndex]) < 0) {
                setUpper = false;
            }
            setBound(recursionBound, boundIndex, newBound, upper);
            plausibleRightBound = Computation::createValidIntervals(boundaries, upperDependencyIndex - 1, newBoundToTest, setUpper, recursionBound);
        }

        if (lowerDependencyIndex <= 0 && upperDependencyIndex <= 0) {
            if (plausibleLeftBound && plausibleRightBound) {
                setBound(recursionBound, boundIndex, newBound, upper);
                return true;
            } else if (!plausibleLeftBound && plausibleRightBound && upper == false) {
                return true;
            } else if (plausibleLeftBound && !plausibleRightBound && upper) {
                return true;
            }
        }
        return plausibleLeftBound && plausibleRightBound;
    }

    bool Computation::isUpper(std::vector<double> first, std::vector<double> second, int index) {
        if (first[index] == 0 && first[0] == 0 && second[0] == 0) {
            return false;
        } else {
            return first[index] - second[index] > 0 ? true : false;
        }
    }


}
