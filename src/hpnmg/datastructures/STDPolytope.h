#ifndef HPNMG_STDPOLYTOPE_H
#define HPNMG_STDPOLYTOPE_H

#include <iostream>
#include <memory>
#include <utility>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>
#include <Eigen/Geometry>
#include <representations/GeometricObject.h>
#include <util/Plotter.h>

namespace hpnmg {
    typedef std::vector<carl::Interval<double>> Intervals;

    template<typename Numeric>
    class STDPolytope {
    public:
        using Polytope = hypro::HPolytope<Numeric>;

        static STDPolytope Empty();
        STDPolytope() = default;
        explicit STDPolytope(const Polytope& polytope);

        ~STDPolytope() = default;
        STDPolytope(const STDPolytope&) = default;
        STDPolytope& operator=(const STDPolytope&) = default;
        STDPolytope(STDPolytope&&) = default;
        STDPolytope& operator=(STDPolytope&&) = default;

        auto contains(const hypro::Point<Numeric> &point) const;
        auto dimension() const { return this->hPolytope.dimension(); }
        auto empty() const { return this->hPolytope.empty(); }

        void insert(const hypro::Halfspace<Numeric> &halfspace);
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
         * Returns those facets of the STDPolytope that are at its bottom with respect to the time dimension.
         *
         * Note that some of those facets may be open. That is, do not assume that points lying on the facets are part
         * of the STDPolytope.
         *
         * @return A vector of the first hit facets if you were to move from beneath the polytope upwards along the time-axis.
         */
        std::vector<Polytope> getBottomFacets() const;

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
        friend boost::serialization::access;
        template<class Archive>
        void save(Archive& ar, const unsigned int version) const {
            const auto polyToData = [](const Polytope& polytope) {
                const auto repr = polytope.inequalities();
                auto matrixData = std::vector<double>(repr.first.size(), 0.0);
                Eigen::VectorXd::Map(matrixData.data(), repr.first.size()) = repr.first;
                auto offsetData = std::vector<double>(repr.second.size(), 0.0);
                Eigen::VectorXd::Map(offsetData.data(), repr.second.size()) = repr.second;

                return std::make_pair(matrixData, offsetData);
            };

            ar << polyToData(this->hPolytope);

            auto openFacetsData = std::vector<std::pair<std::vector<double>, std::vector<double>>>();
            openFacetsData.reserve(this->openFacets.size());
            for (const auto& facet : this->openFacets)
                openFacetsData.push_back(polyToData(facet));
            ar << openFacetsData;
        }
        template<class Archive>
        void load(Archive& ar, const unsigned int version) {
            const auto dataToPoly = [](std::pair<std::vector<double>, std::vector<double>> data) {
                return Polytope(
                    Eigen::MatrixXd::Map(data.first.data(), data.first.size()),
                    Eigen::VectorXd::Map(data.second.data(), data.second.size())
                );
            };

            std::pair<std::vector<double>, std::vector<double>> hPolyData;
            ar >> hPolyData;

            std::vector<std::pair<std::vector<double>, std::vector<double>>> openFacetsData;
            ar >> openFacetsData;
            this->openFacets.reserve(openFacetsData.size());
            for (const auto& facetData : openFacetsData)
                this->openFacets.push_back(dataToPoly(facetData));
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()
        STDPolytope(const Polytope& polytope, const std::vector<Polytope>& openFacets);

        mutable Polytope hPolytope = Polytope();
        std::vector<Polytope> openFacets = {};
    };
}

template<typename Numeric>
std::ostream& operator<<(std::ostream &os, const hpnmg::STDPolytope<Numeric> &region);

#include "datastructures/STDPolytope.tpp"
#endif //HPNMG_STDPOLYTOPE_H