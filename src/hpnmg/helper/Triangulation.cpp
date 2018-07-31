#include "Triangulation.h"

namespace hpnmg {

    std::vector<double> getPointRepresentation(Point<double> p) {
        Eigen::VectorXd v = p.rawCoordinates();
        std::vector<double> coords(v.data(), v.data() + v.rows() * v.cols());
        return coords;
    }

    std::vector<DT::Point> pointsToDTPoints(std::vector<Point<double>> points) {
        std::vector<DT::Point> dTPoints;
        for (Point<double> p : points) {
            std::vector<double> pr = getPointRepresentation(p);
            DT::Point pDT(pr.begin(), pr.end());
            dTPoints.push_back(pDT);
        }
        return dTPoints;
    }

    std::vector<Point<double>> dTPointsToPoints(std::vector<DT::Point> dTPoints) {
        std::vector<Point<double>> points;
        for(DT::Point p : dTPoints) {
            points.push_back(Point<double>(p));
        }
        return points;
    }

    std::vector<DT::Point> regionToDTPoints(Region r) {
        vector<Point<double>> vertices = r.vertices();
        return pointsToDTPoints(vertices);
    }

    Region dTPointsToRegion(std::vector<DT::Point> dTPoints) {
        std::vector<Point<double>> points = dTPointsToPoints(dTPoints);
        return STDiagram::createRegionForVertices(points);
    }

    Triangulation::Triangulation(const ParametricLocationTree::Node &node) : Triangulation(node.getRegion()) {

    }

    Triangulation::Triangulation(const Region &r) {

        DT dt(r.dimension());
        std::vector<DT::Point> points = regionToDTPoints(r);
        dt.insert(points.begin(), points.end());

        std::vector<Region> regions;
        typedef DT::Finite_full_cell_iterator Finite_full_cell_iterator;
        int i = 0;
        for(Finite_full_cell_iterator cit = dt.finite_full_cells_begin();
            cit != dt.finite_full_cells_end(); ++cit )
        {
            std::vector<DT::Point> points;
            for (auto vi = cit->vertices_begin(); vi != cit->vertices_end(); ++vi) {
                DT::Vertex_handle t = *vi;
                points.push_back(t->point());
            }
            Region tr = dTPointsToRegion(points);
            regions.push_back(tr);
        }
        this->t = regions;
    }

    std::vector<Region> Triangulation::getObjects() {
        return this->t;
    }

    std::vector<Region> Triangulation::getSplitVertical() {
        if (this->t_split.size() > 0) {
            return this->t_split;
        }


    }
}

