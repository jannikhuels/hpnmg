#pragma once

#include <memory>

#include "Eigen/Geometry"
#include "representations/GeometricObject.h"

namespace hpnmg {
    typedef std::vector<carl::Interval<double>> Intervals;

    class Region {
    public:
        using PolytopeT = hypro::HPolytope<double>;

        static Region Empty();

        Region() = default;

        std::vector<std::pair<std::vector<double>, std::vector<double>>> getIntegrationIntervals() const;
        Eigen::MatrixXd getIntegrationTransformationMatrix() const;

        //region Region<->PolytopeT conversion
#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-explicit-constructor"
        Region(const PolytopeT& polytope);
        operator const PolytopeT &() const;
#pragma clang diagnostic pop
        //endregion Region<->PolytopeT conversion

        PolytopeT hPolytope = PolytopeT();

    private:
        void computeIntegrationFields() const;

        mutable std::shared_ptr<std::vector<std::pair<std::vector<double>, std::vector<double>>>> integrationIntervals = nullptr;
        mutable std::shared_ptr<Eigen::MatrixXd> integrationTransformationMatrix = nullptr;
    };
};