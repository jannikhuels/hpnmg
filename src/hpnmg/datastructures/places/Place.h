#pragma once

#include <string>

#include <boost/serialization/access.hpp>

namespace hpnmg {
    class Place {
    public:
        std::string id;
        Place(std::string id);

    private:
        friend boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            ar & this->id;
        }
    };
}
