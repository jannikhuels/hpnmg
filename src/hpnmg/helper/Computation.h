#ifndef HPNMG_COMPUTATION_H
#define HPNMG_COMPUTATION_H

#include <vector>

namespace hpnmg {
    class Computation {
    public:

        // Computes the comparison of 2 linear functions
        // always solves for the last variable
        //
        // Example:
        // Linear functions: f1 = 2 + 1 * s1 + 2 * s2, f2 = -1 * s1 + s2
        // Input: time1: [2, 1, 2], time2: [0, -1, 1]
        // Return: [-2, -2, 0]
        // Result : s2 <= -2 -2 s1
        static std::vector<double> computeUnequationCut(std::vector<double> time1, std::vector<double> time2);

        // Computes the comparison of 2 linear functions resolved to a given index
        //
        // Example:
        // Linear functions: f1 = 2 + 1 * s1 + 2 * s2, f2 = -1 * s1 + s2
        // Input: time1: [2, 1, 2], time2: [0, -1, 1], index 1
        // Return: [-1, 0, 1/2]
        // Result : s1 <= -1 -1/2 s1
        static std::vector<double> computeUnequationCut(std::vector<double> time1, std::vector<double> time2, int index);


        // Solve the system of linear equation
        //
        // Example:
        // Linear functions: f1 = -2 * s1 + 2 * s2, f2 = 20 - 2 * s2
        // Input: times: {[0, -2, 2], [20, 0, -2]}, value 4
        // Return: {<{},{-2,0,1}>, <{2,1,0},{8,0,0}>}
        static std::vector<std::vector<std::pair<std::vector<double>, std::vector<double>>>> solveEquations
                (std::vector<std::vector<double>> time, double value);
        
        // Replace values with their corresponding solution
        //
        // Example:
        // Linear functions: s1 = -2 + 1 * s2, s2 = 8
        // Input: times: {<{},{-2,0,1}>, <{2,1,0},{8,0,0}>}
        // Return: {<{},{6,0,0}>, <{2,1,0},{8,0,0}>}
        static std::vector<std::pair<std::vector<double>, std::vector<double>>> replaceValues (std::vector<std::pair<std::vector<double> , std::vector<double>>> in);

        // Insert values into equation
        //
        // Example:
        // Linear function: f1 = -2 + 1 * s2, s2 = 8 + s1
        // Input: func: {-2,0,1}, rep: {8, 1, 0}, index: 2
        // Return: {6, 1, 0}
        static std::vector<double> replace(std::vector<double> func, std::vector<double> repl, int index);

        // Verifies if first equation < second equation
        //
        // Example:
        // Linear function: s1 = s1, s1 = 8, t=4, index = 0
        // Input: func: {0,1}, rep: {8,0}, t: 4, i: 0
        // Return: true
        static bool isSmaller(std::vector<double> f1, std::vector<double> f2, int time);

        // Verifies if first equation > second equation
        //
        // Example:
        // Linear function: s1 = s1, s1 = 8, t=4, index = 0
        // Input: func: {0,1}, rep: {8,0}, t: 4, i: 0
        // Return: false
        static bool isGreater(std::vector<double> f1, std::vector<double> f2, int time);

        static double getTime(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, std::vector<double> dependencies, int mode);

        // Verifies if a given bound is valid (i.e. it reduces to a equation that is valid) and if it should be set (i.e. it is >= than the former lower bound and <= than the former upper bound)
        //
        // Return: {valid, shouldBeSet}
        static std::pair<bool, bool> isValidBound(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper = false);

        static std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> setBound(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper = false);

        static std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> setBoundNoSplit(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper = false);

        static std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> repairIntervals(std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> intervals, int maxIndex = -1);

        static std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> repairInterval(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>, int maxIndex=-1);

        static bool isUpper(std::vector<double> first, std::vector<double> second, int index);

        static int getDependencyIndex(std::vector<double> in1, std::vector<double> in2);

        static int getDependencyIndex(std::vector<double> in);

        static bool createValidIntervals(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper, std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> &recursionBound);

        static std::vector<std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>>> setBoundWithSimpleSplit(std::vector<std::pair<int, std::pair<std::vector<double>, std::vector<double>>>> boundaries, int boundIndex, std::vector<double> newBound, bool upper = false);
    };
}


#endif //HPNMG_COMPUTATION_H
