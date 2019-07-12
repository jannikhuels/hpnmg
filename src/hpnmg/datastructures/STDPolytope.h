#pragma once

#include <iostream>
#include <memory>

#include "Eigen/Geometry"
#include "representations/GeometricObject.h"
#include "util/Plotter.h"

namespace hpnmg {
    typedef std::vector<carl::Interval<double>> Intervals;

    class STDPolytope {
    public:
        using Polytope = hypro::HPolytope<double>;

        static STDPolytope Empty();
        STDPolytope() = default;
        explicit STDPolytope(const Polytope& polytope);

        ~STDPolytope() = default;
        STDPolytope(const STDPolytope&) = default;
        STDPolytope& operator=(const STDPolytope&) = default;
        STDPolytope(STDPolytope&&) = default;
        STDPolytope& operator=(STDPolytope&&) = default;

        auto contains(const hypro::Point<double> &point) const;
        auto dimension() const { return this->hPolytope.dimension(); }
        auto empty() const { return this->hPolytope.empty(); }

        void insert(const hypro::Halfspace<double> &halfspace);
        STDPolytope intersect(const STDPolytope& other) const;
        /**
         * Returns a vector of (possibly overlapping) STDPolytopes whose union represents all points of <code>this</code>
         * that are not in <code>other</code>.
         *
         * @param other The STDPolytope that is to be "subtracted" from this
         * @return The set difference <code>this \ other</code>. Represented as union of (overlapping) STDPolytopes.
         */
        std::vector<STDPolytope> setDifference(const STDPolytope &other) const;
        Polytope timeSlice(double atTime) const;

        /**
         * Add this region's vertices to the plot.
         *
         * Due to open facets, some plotted vertices may not actually be part of the region -- strictly speaking.
         *
         * @param plotter
         */
        void addToPlot(hypro::Plotter<double> &plotter) const;

        /**
         * Print a somewhat Wolfram-Mathematica compatible representation of this region's polytope to the provided ostream.
         *
         * @param os Pass std::cout to print to console.
         */
        void printForWolframMathematica(std::ostream &os) const;
    private:
        STDPolytope(const Polytope& polytope, const std::vector<Polytope>& openFacets);

        Polytope hPolytope = Polytope();
        std::vector<Polytope> openFacets = {};
    };
}

std::ostream& operator<<(std::ostream &os, const hpnmg::STDPolytope &region);
