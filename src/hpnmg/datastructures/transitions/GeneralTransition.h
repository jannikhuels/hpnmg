#pragma once

#include "Transition.h"

namespace hpnmg {
    using namespace std;
    class GeneralTransition : public Transition {

    private:
        unsigned long priority;
        float weight;
        string cdf;
    public:
        const string &getCdf() const;

        void setCdf(const string &cdf);

        const map<string, float> &getParameter() const;

        void setParameter(const map<string, float> &parameter);

    private:
        map<string, float> parameter;
        string policy;

    public:
        GeneralTransition(string id, unsigned long priority, float weight, string cdf, map<string, float> parameter,
                          string policy);
    };
}