//
// Created by Jannik Hüls on 15.01.19.
//

#ifndef HPNMG_LINEARDOMAIN_H
#define HPNMG_LINEARDOMAIN_H

#include "LinearEquation.h"


namespace hpnmg {

    typedef std::pair<LinEq, LinEq> LinearBound;
    typedef std::vector<LinearBound> Domain;
    typedef std::vector<std::pair<int, LinearBound>> DomainWithIndex;

    class LinearDomain {
    private:
        bool alwaysTrue;
        Domain localDomain;
        bool shouldSet(LinearEquation lin);
        std::pair<bool,bool> shouldSet(LinearBound bound);
        bool isTrue(LinearEquation lin);
        void init(LinearEquation init, Domain comparison);

    public:
        LinearDomain(LinearEquation init, Domain comparison);
        LinearDomain(LinearEquation initOne, LinearEquation initTwo, Domain comparison);
        LinearDomain(Domain lin);
        bool isValid();
        bool isEmpty();
        void addEquation(LinearEquation linEq);
        Domain getDomain();
        bool intersect(LinearDomain lin);
        void fill(Domain lin);
        bool equals(Domain comparison);
        void setEquation(LinearEquation lin);
        void setSingleEquation(LinearEquation lin);

        DomainWithIndex toDomainWithIndex(DomainWithIndex dwi);

        static Domain createDomain(DomainWithIndex dwi);
    };
}


#endif //HPNMG_LINEARDOMAIN_H
