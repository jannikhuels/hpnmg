#include "Formula.h"

#include <memory>
#include <utility>

#include "modelchecker/Conjunction.h"
#include "modelchecker/ContinuousAtomicProperty.h"
#include "modelchecker/DiscreteAtomicProperty.h"
#include "modelchecker/Negation.h"
#include "modelchecker/Until.h"

namespace hpnmg {
    Formula::Formula(std::shared_ptr<::hpnmg::Conjunction> conj) : type(Type::Conjunction), conjunction(std::move(conj)) {}
    Formula::Formula(std::shared_ptr<::hpnmg::ContinuousAtomicProperty> cap) : type(Type::ContinuousAtomicProperty), continuousAtomicProperty(std::move(cap)) {}
    Formula::Formula(std::shared_ptr<::hpnmg::DiscreteAtomicProperty> dap) : type(Type::DiscreteAtomicProperty), discreteAtomicProperty(std::move(dap)) {}
    Formula::Formula(std::shared_ptr<::hpnmg::False>) : type(Type::False) {}
    Formula::Formula(std::shared_ptr<::hpnmg::Negation> neg) : type(Type::Negation), negation(std::move(neg)) {}
    Formula::Formula(std::shared_ptr<::hpnmg::True>) : type(Type::True) {}
    Formula::Formula(std::shared_ptr<::hpnmg::Until> until) : type(Type::Until), until(std::move(until)) {}

    Formula::Type Formula::getType() const { return this->type; }

    std::shared_ptr<::hpnmg::Conjunction> Formula::getConjunction() const {
        return this->conjunction;
    }

    std::shared_ptr<::hpnmg::ContinuousAtomicProperty> Formula::getContinuousAtomicProperty() const {
        return this->continuousAtomicProperty;
    }

    std::shared_ptr<::hpnmg::DiscreteAtomicProperty> Formula::getDiscreteAtomicProperty() const {
        return this->discreteAtomicProperty;
    }

    std::shared_ptr<::hpnmg::Negation> Formula::getNegation() const {
        return this->negation;
    }

    std::shared_ptr<::hpnmg::Until> Formula::getUntil() const {
        return this->until;
    }

    std::pair<bool, Formula> Formula::isTrivial() const {
        switch (this->getType()) {
            case Type::Conjunction: {
                const auto lhsTrivial = this->conjunction->left.isTrivial();
                if (!lhsTrivial.first)
                    return lhsTrivial;

                const auto rhsTrivial = this->conjunction->right.isTrivial();
                if (!rhsTrivial.first)
                    return rhsTrivial;

                if (lhsTrivial.second.getType() == Type::False)
                    return lhsTrivial;
                else
                    return rhsTrivial;
            }
            case Type::ContinuousAtomicProperty:
                return {false, *this};
            case Type::DiscreteAtomicProperty:
                return {false, *this};
            case Type::False:
                return {true, *this};
            case Type::Negation: {
                const auto innerTrivial = this->negation->formula.isTrivial();
                if (!innerTrivial.first)
                    return innerTrivial;

                if (innerTrivial.second.getType() == Type::False)
                    return {true, Formula(std::make_shared<True>())};
                else
                    return {true, Formula(std::make_shared<False>())};
            }
            case Type::True:
                return {true, *this};
            case Type::Until:
                return this->until->goal.isTrivial();
        }
    }
}
