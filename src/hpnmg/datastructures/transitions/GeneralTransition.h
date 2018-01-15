#pragma once

#include "Transition.h"

using namespace std;
namespace hpnmg {
    class GeneralTransition : public Transition {

    private:
        unsigned long priority;
        float weight;
        string cdf;
        map<string, float> parameter;
        string policy;

    public:
        GeneralTransition(string id, unsigned long priority, float weight, string cdf, map<string, float> parameter,
                          string policy);
    };
}