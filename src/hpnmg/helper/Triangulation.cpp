#include "Triangulation.h"

namespace hpnmg {

    std::vector<double> getPointRepresentation(hypro::Point<double> p) {
        Eigen::VectorXd v = p.rawCoordinates();
        std::vector<double> coords(v.data(), v.data() + v.rows() * v.cols());
        return coords;
    }

    std::vector<DT::Point> pointsToDTPoints(std::vector<hypro::Point<double>> points) {
        std::vector<DT::Point> dTPoints;
        for (hypro::Point<double> p : points) {
            std::vector<double> pr = getPointRepresentation(p);
            DT::Point pDT(pr.begin(), pr.end());
            dTPoints.push_back(pDT);
        }
        return dTPoints;
    }

    std::vector<hypro::Point<double>> dTPointsToPoints(std::vector<DT::Point> dTPoints) {
        std::vector<hypro::Point<double>> points;
        for(DT::Point p : dTPoints) {
            points.push_back(hypro::Point<double>(p));
        }
        return points;
    }

    std::vector<DT::Point> regionToDTPoints(hypro::HPolytope<double> p) {
        vector<hypro::Point<double>> vertices = p.vertices();
        return pointsToDTPoints(vertices);
    }

    std::vector<Simplex> Triangulation::create(const hypro::HPolytope<double> &p) {
        const auto dimension = p.dimension();
        std::vector<DT::Point> polytopePoints = regionToDTPoints(p);

        if (polytopePoints.size() < dimension + 1)
            return {};

        DT dt(dimension);
        dt.insert(polytopePoints.begin(), polytopePoints.end());

        std::vector<Simplex> simplexes;
        typedef DT::Finite_full_cell_iterator Finite_full_cell_iterator;
        int i = 0;
        for(Finite_full_cell_iterator cit = dt.finite_full_cells_begin();
            cit != dt.finite_full_cells_end(); ++cit )
        {
            std::vector<DT::Point> simplexVertices;
            for (auto vi = cit->vertices_begin(); vi != cit->vertices_end(); ++vi) {
                DT::Vertex_handle t = *vi;
                simplexVertices.push_back(t->point());
            }
            simplexes.emplace_back(hypro::VPolytope<double>(dTPointsToPoints(simplexVertices)));
        }
        return simplexes;
    }
}

