#pragma once

#include "Place.h"


namespace hpnmg {
    class DiscretePlace : public Place {

    private:
        unsigned long marking;

    public:
        DiscretePlace(std::string id, unsigned long marking);
        unsigned long getMarking();
    };
}