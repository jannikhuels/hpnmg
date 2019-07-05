#include <utility>

#include "STDPolytope.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

#include "datastructures/Point.h"
#include "Eigen/Geometry"
#include "util/Plotter.h"

#include "STDiagram.h"
#include "Simplex.h"

namespace hpnmg {
    STDPolytope STDPolytope::Empty() { return STDPolytope(Polytope::Empty()); }

    STDPolytope::STDPolytope(const Polytope& polytope) : STDPolytope(polytope, {}) {}

    STDPolytope::STDPolytope(const Polytope& polytope, const std::vector<Polytope>& openFacets) : hPolytope(polytope), openFacets(openFacets) {}

    void STDPolytope::insert(const hypro::Halfspace<double> &plane) {
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

    auto STDPolytope::contains(const hypro::Point<double> &point) const {
        // Check if the point is contained in our polytope at all
        if (!this->hPolytope.contains(point))
            return false;

        // The vertex is truly contained in our polytope if it does not lie on any of the open facets
        return std::none_of(this->openFacets.begin(), this->openFacets.end(), [&point](const STDPolytope::Polytope &facet) {
            return facet.contains(point);
        });
    }

    STDPolytope STDPolytope::intersect(const STDPolytope &other) const {
        auto intersection = this->hPolytope.intersect(other.hPolytope);

        auto newFacets = std::vector<Polytope>();
        newFacets.reserve(this->openFacets.size() + other.openFacets.size());
        for (const auto &facet : this->openFacets) {
            const auto newFacet = intersection.intersect(facet);
            if (!newFacet.empty())
                newFacets.push_back(newFacet);
        }
        newFacets.shrink_to_fit();

        return STDPolytope(intersection, newFacets);
    }

    vector<STDPolytope> STDPolytope::setDifference(const STDPolytope &other) const {
        assert(this->hPolytope.dimension() == other.hPolytope.dimension());
        auto differences = vector<STDPolytope>();
        differences.reserve(other.hPolytope.constraints().size());

        for (const Halfspace<double>& hsp : other.hPolytope.constraints()) {
            auto negated = STDPolytope(*this);
            negated.insert(-hsp);

            // If the intersection is empty or (at most) a plane, skip this hsp
            if (negated.empty() || negated.hPolytope.vertices().size() <= this->dimension())
                continue;

            // Check if the hsp is the support of an open facet in the new polytope
            const auto& satisfaction = negated.hPolytope.satisfiesHalfspace(hsp);
            if (satisfaction.first)
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

    STDPolytope::Polytope STDPolytope::timeSlice(double atTime) const {
        auto timeSlice = STDPolytope::Polytope(this->hPolytope);

        // Intersect the time-hyperplane with our polytope to create the time-slice
        const auto timeHalfspace = STDiagram::createHalfspaceForTime(atTime, this->hPolytope.dimension());
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
                return hpnmg::STDPolytope::Polytope::Empty();

        auto reducedVertices = timeSlice.vertices();
        if (reducedVertices.empty())
            return STDPolytope::Polytope::Empty();

        for (auto &vertex : reducedVertices)
            vertex.reduceDimension(vertex.dimension() - 1);

        return {reducedVertices};
    }

    void STDPolytope::addToPlot(hypro::Plotter<double> &plotter) const {
        plotter.addObject(this->hPolytope.vertices());
    }

    void STDPolytope::printForWolframMathematica(std::ostream &os) const {
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

std::ostream& operator<<(std::ostream &os, const hpnmg::STDPolytope &region) {
    region.printForWolframMathematica(os);
    return os;
}
