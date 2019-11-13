

#include <representations/GeometricObject.h>
#include <util/plotting/Plotter.h>
#include <util/VariablePool.h>
#include <carl/interval/Interval.h>
​
int main()
{
    //#define exact
    ​
#ifdef exact
    using Number = mpq_class;
    using Pol = hypro::PolyT<Number>;
    using Constr = hypro::ConstraintT<Number>;
    using Var = carl::Variable;
​
    auto& vpool = hypro::VariablePool::getInstance();
​
    // original variables
    Var x = vpool.newCarlVariable("x");
    Var y = vpool.newCarlVariable("y");
​
    hypro::CarlPolytope<Number> c1;
​
    // -x <= 0
    c1.addConstraint(Constr(-Pol(x), carl::Relation::LEQ));
    // -2y + x <= 0
    c1.addConstraint(Constr(-Pol(2)*y + Pol(x), carl::Relation::LEQ));
    // 2 + y <= 0
    c1.addConstraint(Constr(-Pol(2) + Pol(y), carl::Relation::LEQ));
    // -7/2 + x <= 0
    c1.addConstraint(Constr(-Pol(7)/Pol(2) + Pol(x), carl::Relation::LEQ));
    // 7/2 - x <= 0
    c1.addConstraint(Constr(Pol(7)/Pol(2) - Pol(x), carl::Relation::LEQ));
​
#else
    ​
    using Number = double;
    using Matrix = hypro::matrix_t<Number>;
    using Vector = hypro::vector_t<Number>;
    ​
    Matrix constraints = Matrix::Zero(5,2);
    constraints << -1,0,1,-2,0,1,1,0,-1,0;
    Vector constants = Vector::Zero(5);
    constants << 0,0,2,3.5,-3.5;
    ​
    hypro::CarlPolytope<Number> c1;
    ​
#endif
    ​
    ​
    auto tmp = c1.make_rectangular();
    ​
    std::cout << "Matrix: " << c1.matrix() << std::endl;
    std::cout << "Vector: " << c1.vector() << std::endl;
    ​
    auto intervals = c1.getIntervals();
    ​
    for(const auto& i : intervals) {
        std::cout << i << std::endl;
    }
    ​
    return 0;
}