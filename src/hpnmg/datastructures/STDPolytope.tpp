#ifndef HPNMG_STDPOLYTOPE_TPP
#define HPNMG_STDPOLYTOPE_TPP
#include "STDPolytope.h"

#include <representations/GeometricObject.h>
#include <util/plotting/Plotter.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

namespace hpnmg {
    template<typename Numeric>
    STDPolytope<Numeric> STDPolytope<Numeric>::Empty(size_t dimension) {
        hypro::vector_t<Numeric> direction = hypro::vector_t<Numeric>::Zero(dimension);
        direction(0) = Numeric(1);
        hypro::Halfspace<Numeric> a(direction, Numeric(-1));
        direction(0) = Numeric(-1);
        hypro::Halfspace<Numeric> b(direction, Numeric(-1));
        STDPolytope::Polytope res(std::vector<hypro::Halfspace<Numeric>>{a,b});
        assert(res.empty() == true);
        return STDPolytope<Numeric>(res);
    }

    template<typename Numeric>
    STDPolytope<Numeric>::STDPolytope(const Polytope& polytope) : STDPolytope(polytope, {}) {}

    template<typename Numeric>
    STDPolytope<Numeric>::STDPolytope(const Polytope& polytope, const std::vector<Polytope>& openFacets) : hPolytope(polytope), openFacets(openFacets) {}

    template<typename Numeric>
    template<typename ToNumeric>
    STDPolytope<Numeric>::operator STDPolytope<ToNumeric>() const {
        const auto& fromFacets = this->openFacets;
        auto toFacets = std::vector<typename STDPolytope<ToNumeric>::Polytope>();
        toFacets.reserve(fromFacets.size());

        std::transform(fromFacets.begin(), fromFacets.end(), std::back_inserter(toFacets), [](STDPolytope<Numeric>::Polytope poly) {
            return convertHPolytope(poly);
        });

        return STDPolytope<ToNumeric>(convertHPolytope(this->hPolytope), toFacets);
    }

    template<typename Numeric>
    void STDPolytope<Numeric>::insert(const hypro::Halfspace<Numeric> &plane) {
        this->hPolytope.insert(plane);

        for (auto &facet : this->openFacets)
            facet.insert(plane);

        //TODO: Doing this here is probably pretty expensive and only necessary to reduce the memory footprint. We could postpone this action.
        this->openFacets.erase(
            std::remove_if(this->openFacets.begin(), this->openFacets.end(), [](const Polytope& facet) {
                return facet.empty();
            }),
            this->openFacets.end()
        );
    }

    template<typename Numeric>
    void STDPolytope<Numeric>::insertOpen(const hypro::Halfspace<Numeric> &plane) {
        // First, insert as usual
        this->insert(plane);

        // Then turn the plane into an open facet
        auto facet = STDPolytope::Polytope(this->hPolytope);
        facet.insert(-plane);
        if (!facet.empty())
            this->openFacets.push_back(facet);
    }

    template<typename Numeric>
    bool STDPolytope<Numeric>::empty() const {
        if(this->hPolytope.empty())
            return true;

        return std::any_of(this->openFacets.begin(), this->openFacets.end(), [this](const STDPolytope::Polytope& facet) {
            return facet == this->hPolytope;
        });
    }

    template<typename Numeric>
    bool STDPolytope<Numeric>::contains(const hypro::Point<Numeric> &point) const {
        // Check if the point is contained in our polytope at all
        if (!this->hPolytope.contains(point))
            return false;

        // The vertex is truly contained in our polytope if it does not lie on any of the open facets
        return std::none_of(this->openFacets.begin(), this->openFacets.end(), [&point](const STDPolytope::Polytope &facet) {
            return facet.contains(point);
        });
    }

    template<typename Numeric>
    size_t STDPolytope<Numeric>::effectiveDimension() const {
        const auto& vertices = this->hPolytope.vertices();
        if (vertices.empty())
            return 0;

        // taken from hypro::effectiveDimension(). calling it directly gave me linker errors.
        long maxDim = vertices.begin()->rawCoordinates().rows();
        hypro::matrix_t<Numeric> matr = hypro::matrix_t<Numeric>(vertices.size()-1, maxDim);
        // use first vertex as origin, start at second vertex
        long rowIndex = 0;
        for(auto vertexIt = ++vertices.begin(); vertexIt != vertices.end(); ++vertexIt, ++rowIndex) {
            matr.row(rowIndex) = (vertexIt->rawCoordinates() - vertices.begin()->rawCoordinates()).transpose();
        }
        return int(matr.fullPivLu().rank());
    }

    template<typename Numeric>
    STDPolytope<Numeric> STDPolytope<Numeric>::intersect(const STDPolytope &other) const {
        auto intersection = this->hPolytope.intersect(other.hPolytope);

        auto newFacets = std::vector<Polytope>();
        newFacets.reserve(this->openFacets.size() + other.openFacets.size());
        for (const auto &oldFacets : {this->openFacets, other.openFacets}) {
            for (const auto &facet : oldFacets) {
                const auto newFacet = intersection.intersect(facet);
                if (!newFacet.empty())
                    newFacets.push_back(newFacet);
            }
        }
        //TODO: we might want to detect and remove duplicate facets here...
        newFacets.shrink_to_fit();

        return STDPolytope(intersection, newFacets);
    }

    template<typename Numeric>
    std::vector<STDPolytope<Numeric>> STDPolytope<Numeric>::setDifference(const STDPolytope &other) const {
        assert(this->hPolytope.dimension() == other.hPolytope.dimension());
        auto differences = vector<STDPolytope>();
        differences.reserve(other.hPolytope.constraints().size());

        for (const hypro::Halfspace<Numeric>& hsp : other.hPolytope.constraints()) {
            auto negated = STDPolytope(*this);
            negated.insert(-hsp);

            // If the intersection is empty or (at most) a plane, skip this hsp
            if (negated.empty() || negated.hPolytope.vertices().size() <= this->dimension())
                continue;

            // Check if the hsp is the support of an open facet in the new polytope
            const auto& satisfaction = negated.hPolytope.satisfiesHalfspace(hsp);
            if (satisfaction.first != hypro::CONTAINMENT::NO)
                negated.openFacets.push_back(satisfaction.second);

            negated.openFacets.erase(
                std::remove_if(negated.openFacets.begin(), negated.openFacets.end(), [](const Polytope& facet) {
                    return facet.empty();
                }),
                negated.openFacets.end()
            );
            differences.push_back(negated);
        }

        return differences;
    }

    template<typename Numeric>
    std::vector<STDPolytope<Numeric>> STDPolytope<Numeric>::setDifference(const std::vector<STDPolytope> &others) const {
        auto differences = vector<STDPolytope>{*this};
        for (const auto& other : others) {
            auto combinedDifferences = vector<STDPolytope>();
            for (const auto& difference : differences) {
                auto newDifferences = difference.setDifference(other);
                combinedDifferences.insert(combinedDifferences.end(), newDifferences.begin(), newDifferences.end());
            }
            differences = std::move(combinedDifferences);
            differences.erase(
              std::remove_if(differences.begin(), differences.end(), [](const STDPolytope& poly) { return poly.empty(); }),
              differences.end()
            );
        }

        return differences;
    }

    template<typename Numeric>
    typename STDPolytope<Numeric>::Polytope STDPolytope<Numeric>::timeSlice(Numeric atTime) const {
        auto timeSlice = STDPolytope::Polytope(this->hPolytope);

        // Intersect the time-hyperplane with our polytope to create the time-slice
        hypro::vector_t<Numeric> direction = hypro::vector_t<Numeric>::Zero(this->hPolytope.dimension());
        direction[this->hPolytope.dimension() - 1] = 1;
        const auto timeHalfspace = hypro::Halfspace<Numeric>(direction, atTime);
        timeSlice.insert(timeHalfspace);
        timeSlice.insert(-timeHalfspace);

        // Check if the time-hyperplane was actually a supporting hyperplane for one of the open facets:
        //    Usually, we'd check for mutual containment of the two polytopes, but that returned the wrong result.
        //    Possibly due to numeric imprecision?
        //    However, from how the time-slice and the open facets are constructed, we know that neither can contain the
        //    other without them being equal. Thus, if the number of vertices of their intersection is the same as
        //    before the intersection, they must be equal.
        for (const auto& openFacet : this->openFacets)
            if (openFacet.intersect(timeSlice).vertices().size() == timeSlice.vertices().size())
                return STDPolytope::Empty(this->dimension()).hPolytope;

        auto reducedVertices = timeSlice.vertices();
        if (reducedVertices.empty())
            return STDPolytope::Empty(this->dimension()).hPolytope;

        for (auto &vertex : reducedVertices)
            vertex.reduceDimension(vertex.dimension() - 1);

        for (const auto& vertices : {this->hPolytope.vertices(), timeSlice.vertices(), reducedVertices}) {
            long maxDim = vertices.begin()->rawCoordinates().rows();
            hypro::matrix_t<Numeric> matr = hypro::matrix_t<Numeric>(vertices.size()-1, maxDim);
            // use first vertex as origin, start at second vertex
            long rowIndex = 0;
            for(auto vertexIt = ++vertices.begin(); vertexIt != vertices.end(); ++vertexIt, ++rowIndex) {
                matr.row(rowIndex) = (vertexIt->rawCoordinates() - vertices.begin()->rawCoordinates()).transpose();
            }
            auto effectiveDimension = int(matr.fullPivLu().rank());
            std::cout << "(vertex dim, effective dim) = (" << vertices[0].dimension() << ", " << effectiveDimension << ")" << std::endl;
        }

        return {reducedVertices};
    }

    template<typename Numeric>
    typename STDPolytope<Numeric>::Polytope STDPolytope<Numeric>::extendDownwards() const {
        auto vertices = this->hPolytope.vertices();
        if (vertices.empty())
            return STDPolytope<Numeric>::Empty(this->dimension()).hPolytope;

        // The vector must not reallocate (and thus invalidate its iterators) while we duplicate it
        vertices.reserve(vertices.size() * 2);
        std::transform(vertices.begin(), vertices.end(), std::back_inserter(vertices), [](hypro::Point<Numeric> vertex) {
            vertex[vertex.dimension() - 1] = 0;
            return vertex;
        });

        return hypro::HPolytope<Numeric>(vertices);
    }

    template<typename Numeric>
    void STDPolytope<Numeric>::addToPlot(hypro::Plotter<Numeric> &plotter) const {
        plotter.addObject(this->hPolytope.vertices());
    }

    template<typename Numeric>
    void STDPolytope<Numeric>::printForWolframMathematica(std::ostream &os) const {
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

}

template<typename Numeric>
std::ostream& operator<<(std::ostream &os, const hpnmg::STDPolytope<Numeric> &region) {
    region.printForWolframMathematica(os);
    return os;
}
#endif //HPNMG_STDPOLYTOPE_TPP