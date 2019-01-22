//
// Created by Jannik Hüls on 15.01.19.
//

#include "LinearDomain.h"

namespace hpnmg {

    void LinearDomain::init(hpnmg::LinearEquation linearEquation, hpnmg::Domain comparison) {
        if (comparison.size() > 0) {
            this->localDomain.push_back(comparison[0]);
            if (shouldSet(linearEquation)) {
                this->setEquation(linearEquation);
            } else {
                if (!this->isTrue(linearEquation)) {
                    this->localDomain[0] = {{},
                                            {}};
                }
            }
        }
    }

    LinearDomain::LinearDomain(LinearEquation linearEquation, Domain comparison) {
        this->init(linearEquation, comparison);
    }

    LinearDomain::LinearDomain(hpnmg::LinearEquation initOne, hpnmg::LinearEquation initTwo,
                               hpnmg::Domain comparison) {
        this->init(initOne, comparison);

        if(!initTwo.alwaysTrue) {
            if (this->shouldSet(initTwo)) {
                this->setEquation(initTwo);
            } else {
                if (!this->isTrue(initTwo)) {
                    this->localDomain[0] = {{},
                                            {}};
                }
            }
        }
    }

    LinearDomain::LinearDomain(hpnmg::Domain lin) {
        for (LinearBound b : lin) {
            this->localDomain.push_back(b);
        }
    }

    bool LinearDomain::isTrue(hpnmg::LinearEquation lin) {
        if(!this->isValid()) {
            return false;
        }

        if (lin.alwaysTrue) {
            return true;
        }

        if (lin.isUpper) {
            return this->localDomain[0].second[0] <= lin.equation[0];
        } else {
            return lin.equation[0] <= this->localDomain[0].first[0];
        }
    }

    bool LinearDomain::isValid() {
        LinearBound firstBound = this->localDomain[0];
        return (firstBound.first.size() > 0 && firstBound.second.size() > 0) && firstBound.first[0] >= 0 &&
               firstBound.second[0] >= firstBound.first[0] && firstBound.first[0] != firstBound.second[0];
    }

    bool LinearDomain::intersect(LinearDomain lin) {
        std::pair<bool, bool> setIt = this->shouldSet(lin.localDomain[0]);
        if (setIt.first || setIt.second) {

            if (setIt.first) {
                this->setEquation(LinearEquation(lin.localDomain[0].first, false, 0));
            }
            if (setIt.second) {
                this->setEquation(LinearEquation(lin.localDomain[0].second, true, 0));
            }

            if (lin.localDomain.size() > 1) {
                int i = 1;
                for (; i < this->localDomain.size(); i++) {
                    if (lin.localDomain[i].first.size() > 0) {
                        this->localDomain[i].first = lin.localDomain[i].first;
                    }
                    if (lin.localDomain[i].second.size() > 0) {
                        this->localDomain[i].second = lin.localDomain[i].second;
                    }
                }

                if (lin.localDomain.size() >= i) {
                    for (; i < lin.localDomain.size(); i++) {
                        this->localDomain.push_back(lin.localDomain[i]);
                    }
                }
            }
            return true;
        } else if (lin.isValid()) {
            if (lin.localDomain[0].first[0] <= this->localDomain[0].first[0] && lin.localDomain[0].second[0] >= this->localDomain[0].second[0]) {
                return true;
            }
        }
        return false;
    }

    bool LinearDomain::shouldSet(LinearEquation eq) {
        if (this->isValid()) {
            LinearBound firstBound = this->localDomain[0];
            return (firstBound.first[0] < eq.equation[0]) && (eq.equation[0] < firstBound.second[0]);
        }
        return false;
    }

    std::pair<bool,bool> LinearDomain::shouldSet(LinearBound bound) {
        return {this->shouldSet(LinearEquation(bound.first, false, 0)), shouldSet(LinearEquation(bound.second, true, 0))};
    }

    void LinearDomain::addEquation(hpnmg::LinearEquation linEq) {
        if (linEq.isUpper) {
            this->localDomain.push_back({{}, linEq.equation});
        } else {
            this->localDomain.push_back({linEq.equation, {}});
        }
    }

    void LinearDomain::setEquation(hpnmg::LinearEquation lin) {
        if (this->localDomain.size() > lin.dependencyIndex) {
            if (lin.isUpper) {
                this->localDomain[lin.dependencyIndex].second = lin.equation;
            } else {
                this->localDomain[lin.dependencyIndex].first = lin.equation;
            }
        }
    }

    Domain LinearDomain::getDomain() {
        return this->localDomain;
    }

    void LinearDomain::fill(Domain lin) {
        for (int i = 0; i < lin.size(); i++) {
            if (i < this->localDomain.size()) {
                if (this->localDomain[i].first.size() == 0) {
                    this->localDomain[i].first = lin[i].first;
                }
                if (this->localDomain[i].second.size() == 0) {
                    this->localDomain[i].second = lin[i].second;
                }
            } else {
                this->localDomain.push_back(lin[i]);
            }
        }
    }

    bool equalBound(LinearBound b1, LinearBound b2) {
        if (b1.first.size() == b1.second.size()) {
            if (b1.first.size() == b2.first.size() && b1.second.size() == b2.second.size()) {
                for(int i = 0; i < b1.first.size(); i++) {
                    if(b1.first[i] != b2.first[i] || b1.second[i] != b2.second[i]) {
                        return false;
                    }
                }
                return true;
            }
        }
        return false;
    }

    bool LinearDomain::equals(hpnmg::Domain comparison) {
        if (!(this->localDomain.size() == comparison.size())) {
            return false;
        }

        for (int i = 0; i < this->localDomain.size(); i++) {
            if (!equalBound(this->localDomain[i], comparison[i])) {
                return false;
            }
        }
        return true;
    }

    DomainWithIndex LinearDomain::toDomainWithIndex(hpnmg::DomainWithIndex dwi) {
        assert(dwi.size() == this->localDomain.size());
        for (int i = 0; i < dwi.size(); i++) {
            dwi[i].second = this->localDomain[i];
        }
        return dwi;
    }

    Domain LinearDomain::createDomain(hpnmg::DomainWithIndex dwi) {
        Domain d;
        for (std::pair<int, LinearBound> b : dwi) {
            d.push_back(b.second);
        }
        return d;
    }
}