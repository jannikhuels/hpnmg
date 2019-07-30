#ifndef HPNMG_STDPOLYTOPE_H
#define HPNMG_STDPOLYTOPE_H

#include <iostream>
#include <memory>

#include <gmpxx.h>

#include <Eigen/Geometry>
#include <representations/GeometricObject.h>
#include <util/plotting/Plotter.h>

namespace hpnmg {
    typedef std::vector<carl::Interval<double>> Intervals;

    hypro::HPolytope<double> convertHPolytope(const hypro::HPolytope<mpq_class>& from);
    hypro::HPolytope<mpq_class> convertHPolytope(const hypro::HPolytope<double>& from);

    template<typename Numeric>
    class STDPolytope {
    public:
        using Polytope = hypro::HPolytope<Numeric>;

        static STDPolytope Empty();
        STDPolytope() = default;
        explicit STDPolytope(const Polytope& polytope);
        STDPolytope(const Polytope& polytope, const std::vector<Polytope>& openFacets);

        ~STDPolytope() = default;
        STDPolytope(const STDPolytope&) = default;
        STDPolytope& operator=(const STDPolytope&) = default;
        STDPolytope(STDPolytope&&) = default;
        STDPolytope& operator=(STDPolytope&&) = default;

        template<typename ToNumeric>
        explicit operator STDPolytope<ToNumeric>() const;

        bool contains(const hypro::Point<Numeric> &point) const;
        size_t dimension() const { return this->hPolytope.dimension(); }
        size_t effectiveDimension() const;
        bool empty() const { return this->hPolytope.empty(); }

        /**
         * Inserts a new half space into the polytope. That is, intersects the underlying polytope and all open facets
         * with the halfspace.
         *
         * @param halfspace
         */
        void insert(const hypro::Halfspace<Numeric> &halfspace);

        /**
         * Like STDPolytope::insert(), but the provided half space is considered to be open. That is, intersects the
         * underlying polytope and all open facets with the halfspace and the halfspace as open facet.
         *
         * @param halfspace
         */
        void insertOpen(const hypro::Halfspace<Numeric> &halfspace);
        STDPolytope intersect(const STDPolytope& other) const;
        /**
         * Returns a vector of (possibly overlapping) STDPolytopes whose union represents all points of <code>this</code>
         * that are not in <code>other</code>.
         *
         * @param other The STDPolytope that is to be "subtracted" from this
         * @return The set difference <code>this \ other</code>. Represented as union of (overlapping) STDPolytopes.
         */
        std::vector<STDPolytope> setDifference(const STDPolytope &other) const;
        /**
         * Returns a vector of (possibly overlapping) STDPolytopes whose union represents all points of <code>this</code>
         * that are not in any of <code>other</code>.
         *
         * @param other The STDPolytopes that are to be "subtracted" from this
         * @return The set difference <code>this \ (union over others)</code>. Represented as union of (overlapping)
         *         STDPolytopes.
         */
        std::vector<STDPolytope> setDifference(const std::vector<STDPolytope> &others) const;
        Polytope timeSlice(Numeric atTime) const;

        Polytope extendDownwards() const;

        /**
         * Add this region's vertices to the plot.
         *
         * Due to open facets, some plotted vertices may not actually be part of the region -- strictly speaking.
         *
         * @param plotter
         */
        void addToPlot(hypro::Plotter<Numeric> &plotter) const;

        /**
         * Print a somewhat Wolfram-Mathematica compatible representation of this region's polytope to the provided ostream.
         *
         * @param os Pass std::cout to print to console.
         */
        void printForWolframMathematica(std::ostream &os) const;
    private:
        mutable Polytope hPolytope = Polytope();
        std::vector<Polytope> openFacets = {};
    };
}

template<typename Numeric>
std::ostream& operator<<(std::ostream &os, const hpnmg::STDPolytope<Numeric> &region);

// If we want users of the library to be able to choose the <code>Numeric</code> type for STDPolytpe, the template
// definitions *need* to be in this header.
// However, this will greatly increase compilation time during development of hpnmg because for every change to the
// implementation, i.e. the template definitions, every file including the header needs to be recompiled.
// Instead, the template file is commented out and the template is explicitly instantiated in STDPolytope.cpp for the
// types we actually use during development.
//#include "datastructures/STDPolytope.tpp"
#endif //HPNMG_STDPOLYTOPE_H