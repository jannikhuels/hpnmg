#include "DiscretePlace.h"

namespace hpnmg {

    int DiscretePlace::getNumberOfMarkings() const {
        return numberOfMarkings;
    }

    void DiscretePlace::setNumberOfMarkings(int numberOfMarkings) {
        DiscretePlace::numberOfMarkings = numberOfMarkings;
    }
}
