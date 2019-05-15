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

    /*std::vector<Point<double>> dTPointsToPoints(std::vector<DT::Point> dTPoints) {
        std::vector<Point<double>> points;
        for(DT::Point p : dTPoints) {
            points.push_back(Point<double>(p));
        }
        return points;
    }*/

    std::vector<DT::Point> regionToDTPoints(Region r) {
        vector<Point<double>> vertices = r.vertices();
        return pointsToDTPoints(vertices);
    }

    /*Region dTPointsToRegion(std::vector<DT::Point> dTPoints) {
        std::vector<Point<double>> points = dTPointsToPoints(dTPoints);
        return STDiagram::createRegionForVertices(points);
    }*/

    Triangulation::Triangulation(const ParametricLocationTree::Node &node) : Triangulation(node.getRegion()) {

    }

    Triangulation::Triangulation(const Region &r) {

        DT dt(r.dimension());
        std::vector<DT::Point> points = regionToDTPoints(r);
        dt.insert(points.begin(), points.end());

        std::vector<Region> regions;
        typedef DT::Finite_full_cell_iterator Finite_full_cell_iterator;


        for(Finite_full_cell_iterator cit = dt.finite_full_cells_begin();
            cit != dt.finite_full_cells_end(); ++cit )
        {
            std::vector<Point<double>> points;
            for (auto vi = cit->vertices_begin(); vi != cit->vertices_end(); ++vi) {
                DT::Vertex_handle t = *vi;
                vector_t<double> p = vector_t<double>::Zero(r.dimension());
                int index = 0;
                for (auto i = t->point().begin(); i != t->point().end(); ++i) {
                    p[index] = (double)*i;
                    index++;
                }
                points.push_back(Point<double>(p));
            }

            Region tr(points);
            regions.push_back(tr);
        }
        this->t = regions;
    }

    std::vector<Region> Triangulation::getObjects() {
        return this->t;
    }

    Point<double> getCentralPoint(Region r, int dim_index) {
        r.project({(unsigned long)dim_index, (unsigned long)dim_index+1});

        std::vector<Point<double>> vertices = r.vertices();

        if (vertices[0][0] < vertices[1][0]) {
            if (vertices[1][0] < vertices[2][0]) {
                return vertices[1];
            } else if (vertices[0][0] < vertices[2][0]) {
                return vertices[2];
            }
            return vertices[0];
        } else {
            if (vertices[1][0] > vertices[2][0]) {
                return vertices[1];
            } else if (vertices[0][0] > vertices[2][0]) {
                return vertices[2];
            }
            return vertices[0];
        }
    }

    Region makeCompleteRegion(std::vector<Point<double>> base, Region rest, int dim_index) {

        std::vector<Point<double>> points;
        points.insert(points.end(), base.begin(), base.end());
        int dimension = rest.dimension();

        std::vector<size_t> restDim;
        for (int i = 0; i < rest.dimension(); i++){
            if (i==dim_index || i==dim_index+1) {
                continue;
            }
            vector_t<double> dir = vector_t<double>::Zero(dimension);
            dir[i] = -1;
            EvaluationResult<double> low = rest.evaluate(dir);
            dir[i] = 1;
            EvaluationResult<double> up = rest.evaluate(dir);

            points.push_back((Point<double>)up.optimumValue);
            points.push_back((Point<double>)low.optimumValue);
        }

        Region r(points);
        return r;
    }

    std::vector<Region> splitRegion(Region r, Point<double> p, int dim_index) {
        int dimension = r.dimension();
        vector_t<double> dir = vector_t<double>::Zero(dimension);
        dir[dim_index] = 1;

        double planeOffset = Halfspace<double>::computePlaneOffset(dir, p);
        Halfspace<double> h(dir, planeOffset);

        Region left(r);
        left.insert(h);

        Region right(r);
        right.insert(-h);

        if (left.vertices().size() == dimension || right.vertices().size() == dimension) {
            return {r};
        }

        left = makeCompleteRegion(left.project({(unsigned long)dim_index, (unsigned long)(dim_index+1)}).vertices(), r, dim_index);
        right = makeCompleteRegion(right.project({(unsigned long)dim_index, (unsigned long)(dim_index+1)}).vertices(), r, dim_index);

        return {left, right};
    }

    std::vector<Region> splitVertical(Region r) {
        // Do not split for the t coordinates
        int max_dim = r.dimension() - 2;

        std::vector<Region> res;
        res.push_back(r);

        for (int i = 0; i < max_dim; i++) {
            for (Region r : res) {
                std::vector<Region> update;
                Point<double> outmost = getCentralPoint(r, i);

                update = splitRegion(r, outmost, i);

                res = update;
            }

        }

        return res;
    }

    std::vector<Region> Triangulation::getSplitVertical() {
        if (this->t_split.size() > 0) {
            return this->t_split;
        }

        std::vector<Region> res;
        for (Region r : this->t) {
            std::vector<Region> split = splitVertical(r);
            res.insert(res.end(), split.begin(), split.end());
        }
        this->t_split = res;

        return res;
    }

//    std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>> bounds(Region r) {
//        // Do not split for the t coordinates
//        int max_dim = r.dimension() - 2;
//
//        std::vector<std::vector<double>> leftBounds;
//        std::vector<std::vector<double>> rightBounds;
//
//        for (int i = 1; i < max_dim; i++) {
//
//        }
//
//        return res;
//    }
//
//    std::vector<std::pair<std::vector<std::vector<double>>, std::vector<std::vector<double>>>> Triangulation::getBounds() {
//        for (Region r : this->t) {
//
//        }
//    }
}

