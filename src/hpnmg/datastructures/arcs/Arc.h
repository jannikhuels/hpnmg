#pragma once

#include <string>
#include "../transitions/Transition.h"
#include "../places/Place.h"


namespace hpnmg {
    class Arc {

    public:
        std::string id;
        float weight;
        Transition transition;
        Place place;
        bool isInputArc;

        Arc(std::string id, float weight, Transition transition, Place place, bool isInputArc);
    };
}
