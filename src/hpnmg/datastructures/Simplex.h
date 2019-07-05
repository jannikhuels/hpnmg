#ifndef HPNMG_SIMPLEX_H
#define HPNMG_SIMPLEX_H


#include <iostream>
#include <memory>
#include "Eigen/Geometry"
#include "representations/GeometricObject.h"

namespace hpnmg {
    class Simplex {
    public:
        explicit Simplex(const hypro::HPolytope<double> &polytope);

        explicit Simplex(hypro::VPolytope<double> polytope);

        Simplex() = delete;
        ~Simplex() = default;
        Simplex(const Simplex&) = default;
        Simplex& operator=(const Simplex&) = default;
        Simplex(Simplex&&) = default;
        Simplex& operator=(Simplex&&) = default;

        auto vertices() const { return this->polytope.vertices(); }

        vector<pair<vector<double>, vector<double>>> getIntegrationIntervals() const;

        Eigen::MatrixXd getIntegrationTransformationMatrix() const;

    private:
        mutable std::shared_ptr<std::vector<std::pair<std::vector<double>, std::vector<double>>>> integrationIntervals = nullptr;
        mutable std::shared_ptr<Eigen::MatrixXd> integrationTransformationMatrix = nullptr;

        void computeIntegrationFields() const;

        hypro::VPolytope<double> polytope;
    };
}

#endif //HPNMG_SIMPLEX_H
