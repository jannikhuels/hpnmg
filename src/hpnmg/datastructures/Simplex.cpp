#include <utility>

#include "Simplex.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

#include "datastructures/Point.h"
#include "Eigen/Geometry"
#include "STDiagram.h"
#include "STDPolytope.h"

namespace hpnmg {
    Simplex::Simplex(VPolytope<double> polytope) : polytope(std::move(polytope)) {
        const auto &vertices = this->polytope.vertices();

        assert(this->polytope.dimension() > 0);
        assert(vertices.size() > 1);

        if (vertices.size() > this->polytope.dimension() + 1)
            throw std::domain_error("Simplex(): polytope has too many vertices to be a simplex");

        // TODO: The polytope describes, for example, a plane in 3D. Can this even happen during our model checking? What do we do then?
        if (vertices.size() < this->polytope.dimension() + 1)
            throw std::domain_error("Simplex(): polytope has too few vertices to be a simplex");
    }

    Simplex::Simplex(const hypro::HPolytope<double> &polytope) : Simplex(hypro::Converter<double>::toVPolytope(polytope)) {}

    vector<pair<vector<double>, vector<double>>> Simplex::getIntegrationIntervals() const {
        if (this->integrationIntervals == nullptr)
            this->computeIntegrationFields();

        return *this->integrationIntervals;
    }

    Eigen::MatrixXd Simplex::getIntegrationTransformationMatrix() const {
        if (this->integrationTransformationMatrix == nullptr)
            this->computeIntegrationFields();

        return *this->integrationTransformationMatrix;
    }

    void Simplex::computeIntegrationFields() const {
        const auto &vertices = this->polytope.vertices();

        auto transMatrix = Eigen::MatrixXd(vertices.size(), vertices.size());
        std::for_each(
            vertices.begin() + 1,
            vertices.end(),
            [&transMatrix, &vertices, col = 0](const auto &vertex) mutable {
                transMatrix.col(col++) << vertex.rawCoordinates() - vertices[0].rawCoordinates(), 0;
            }
        );
        transMatrix.col(transMatrix.cols() - 1) << vertices[0].rawCoordinates(), 1;

        auto intervals = vector<pair<vector<double>, vector<double>>>(
            this->polytope.dimension(),
            {vector<double>(this->polytope.dimension() + 1, 0), vector<double>(this->polytope.dimension() + 1, 0)}
        );

        // For our unit simplex, the intervals are always the same:
        // [0, 1] for S0, [0, 1 - S0] for S1, ..., [0, 1 - S0 - ... SN] for SN+1
        std::for_each(
            intervals.begin(),
            intervals.end(),
            [currentDimension = 0](auto &interval) mutable {
                auto &lowerBound = interval.first;
                lowerBound[currentDimension + 1] = 1;

                auto &upperBound = interval.second;
                upperBound[0] = 1;
                upperBound[currentDimension + 1] = 1;
                fill_n(upperBound.begin() + 1, currentDimension, -1);

                currentDimension++;
            }
        );

        this->integrationTransformationMatrix = make_shared<Eigen::MatrixXd>(transMatrix);
        this->integrationIntervals = make_shared<vector<pair<vector<double>, vector<double>>>>(intervals);
    }
}
