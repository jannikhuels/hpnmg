#pragma once

#include <string>
#include <memory>
#include "../places/Place.h"


namespace hpnmg {
    class Arc {

    public:
        std::string id;
        float weight;
        std::shared_ptr<Place> place;

        Arc(std::string id, float weight, std::shared_ptr<Place> place);
    };
}
