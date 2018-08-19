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
        if (k < 1 || diffK == 0.0)
            return {};
        std::vector<double> unequationCut;
        for (int i = 0; i < minSize; ++i) {
            if (i < k) {
                unequationCut.push_back((time1[i] - time2[i]) / diffK);
            } else {
                unequationCut.push_back(0);
            }
        }


        return unequationCut;
    }

    std::vector<double> Computation::computeUnequationCut(std::vector<double> time1, std::vector<double> time2, int index) {
        // Swap the variables
        int v1 = time1[index];
        time1[index] = time1[time1.size()-1];
        time1[time1.size()-1] = v1;

        int v2 = time2[index];
        time2[index] = time2[time2.size()-1];
        time2[time2.size()-1] = v2;

        std::vector<double> res = computeUnequationCut(time1, time2);

        //Swap back
        double v = res[index];
        res[index] = 0;
        res[res.size()-1] = v;

        return res;
    }

    std::vector<std::pair<std::vector<double>, std::vector<double>>> Computation::solveEquations(std::vector<std::vector<double>> time, int value) {
        std::vector<std::pair<std::vector<double>, std::vector<double>>> res;

        if (time.size() == 0) {
            return res;
        }

        int size = time[0].size();

        // Create time equation
        std::vector<double> t(size);
        std::fill(t.begin(), t.end(), 0);
        t[0] = value;

        for (int i = size - 1; i > 0; i--) {
            std::pair<std::vector<double>, std::vector<double>> p({},{});
            for (std::vector<double> v : time) {
                if (v[i] > 0) {
                    // lower bound
                    p.first = Computation::computeUnequationCut(t, v, i);

                } else if (v[i] < 0) {
                    // upper bound
                    p.second = Computation::computeUnequationCut(t,v,i);
                }
            }
            res.insert(res.begin(), p);
        }
        return res;
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

        if (s2 == 0 && res.first.size() > 0) {
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

            if (s1 != 0) {
                allSolved = false;
                in[i].first = getSolve(in[i].first, in[s1-1], s1, i+1);
            }

            if (s2 != 0) {
                allSolved = false;
                in[i].second = getSolve(in[i].second, in[s2-1], s2, i+1);
            }
        }

        if (allSolved)
            return in;
        return Computation::replaceValues(in);
    }

    std::vector<double> Computation::replace(std::vector<double> func, std::vector<double> repl, int index) {
        int factor = func[index];
        func[index] = 0;
        for (int i = 0; i < func.size(); i++) {
            func[i] = func[i] + factor * repl[i];
        }
        return func;
    }
}
