#pragma once

#include "Transition.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/map.hpp>

using namespace std;
namespace hpnmg {
    class GeneralTransition : public Transition {

    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & boost::serialization::base_object<Transition>(*this);
            ar & this->priority;
            ar & this->weight;
            ar & this->cdf;
            ar & this->parameter;
            ar & this->policy;
        }

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