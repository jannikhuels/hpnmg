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

    std::vector<std::vector<std::pair<std::vector<double>, std::vector<double>>>> Computation::solveEquations(std::vector<std::vector<double>> time, int value) {

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


}
