#include "Region.h"

#include "datastructures/Point.h"

#include "Eigen/Geometry"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

namespace hpnmg {
    Region Region::Empty() { return Region(PolytopeT::Empty()); }

    std::vector<std::pair<std::vector<double>, std::vector<double>>> Region::getIntegrationIntervals() const {
        if (this->integrationIntervals == nullptr)
            this->computeIntegrationFields();

        return *this->integrationIntervals;
    }

    Eigen::MatrixXd Region::getIntegrationTransformationMatrix() const {
        if (this->integrationTransformationMatrix == nullptr)
            this->computeIntegrationFields();

        return *this->integrationTransformationMatrix;
    }

    void Region::printForWolframMathematica(std::ostream &os) const {
        os << "{";
        bool firstHalfspace = true;
        for (const auto &halfspace : this->hPolytope.constraints()) {
            if (!firstHalfspace)
                os << ", ";

            os << "HalfSpace[{";
            const auto &normal = halfspace.normal();
            for (auto i = 0; i < normal.size(); ++i) {
                if (i > 0)
                    os << ", ";
                os << normal[i];
            }
            os << "}, " << halfspace.offset()
                << "]";

            firstHalfspace = false;
        }
        os << "}";
    }

    //region Region<->PolytopeT conversion
    Region::Region(const PolytopeT& polytope) : hPolytope(polytope) {}
    Region::operator const PolytopeT &() const { return hPolytope; }
    //endregion Region<->PolytopeT conversion

    void Region::computeIntegrationFields() const {
        const auto &vertices = this->hPolytope.vertices();

        assert(this->hPolytope.dimension() > 0);
        assert(vertices.size() > 1);

        if (vertices.size() > this->hPolytope.dimension() + 1) {
            //TODO: recursively partition the polytope into simplexes
            std::cerr << "Region::computeIntegrationFields() called for a non-base case polytope." << std::endl;
            std::abort();
        }

        if (vertices.size() < this->hPolytope.dimension() + 1) {
            // TODO: The region describes, for example, a plane in 3D. Can this even happen during our model checking? What do we do then?
            std::cerr << "Region::computeIntegrationFields() called for a region without volume." << std::endl;
            std::abort();
        }

        auto transMatrix = Eigen::MatrixXd(vertices.size(), vertices.size());
        std::for_each(
            vertices.begin() + 1,
            vertices.end(),
            [&transMatrix, &vertices, col = 0](const auto &vertex) mutable {
                transMatrix.col(col++) << vertex.rawCoordinates() - vertices[0].rawCoordinates(), 0;
            }
        );
        transMatrix.col(transMatrix.cols() - 1) << vertices[0].rawCoordinates(), 1;

        auto intervals = std::vector<std::pair<std::vector<double>, std::vector<double>>>(
            this->hPolytope.dimension(),
            {std::vector<double>(this->hPolytope.dimension() + 1, 0), std::vector<double>(this->hPolytope.dimension() + 1, 0)}
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
                std::fill_n(upperBound.begin() + 1, currentDimension, -1);

                currentDimension++;
            }
        );

        this->integrationTransformationMatrix = std::make_shared<Eigen::MatrixXd>(transMatrix);
        this->integrationIntervals = std::make_shared<std::vector<std::pair<std::vector<double>, std::vector<double>>>>(intervals);
    }
}

std::ostream& operator<<(std::ostream &os, const hpnmg::Region &region) {
    region.printForWolframMathematica(os);
    return os;
}
