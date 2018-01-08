#pragma once

#include "Place.h"


namespace hpnmg {
    class DiscretePlace : public Place {

    private:
        int numberOfMarkings;
        int initialMarking;

    public:
        int getNumberOfMarkings() const;
        void setNumberOfMarkings(int numberOfMarkings);
    };
}