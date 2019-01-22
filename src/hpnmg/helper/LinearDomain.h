//
// Created by Jannik Hüls on 15.01.19.
//

#ifndef HPNMG_LINEARDOMAIN_H
#define HPNMG_LINEARDOMAIN_H

#include "LinearEquation.h"


namespace hpnmg {

    typedef std::pair<LinEq, LinEq> LinearBound;
    typedef std::vector<LinearBound> Domain;

    class LinearDomain {
    private:
        bool alwaysTrue;
        Domain localDomain;
        bool shouldSet(LinearEquation lin);
        std::pair<bool,bool> shouldSet(LinearBound bound);
        bool isTrue(LinearEquation lin);
        void setEquation(LinearEquation lin);
        void init(LinearEquation init, Domain comparison);

    public:
        LinearDomain(LinearEquation init, Domain comparison);
        LinearDomain(LinearEquation initOne, LinearEquation initTwo, Domain comparison);
        bool isValid();
        void addEquation(LinearEquation linEq);
        Domain getDomain();
        bool intersect(LinearDomain lin);
        void fill(Domain lin);
        bool equals(Domain comparison);
    };
}


#endif //HPNMG_LINEARDOMAIN_H
