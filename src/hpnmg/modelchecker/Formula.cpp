#include "Formula.h"

#include <memory>
#include <utility>

#include "modelchecker/ContinuousAtomicProperty.h"
#include "modelchecker/DiscreteAtomicProperty.h"

namespace hpnmg {
    class Conjunction;

    Formula::Formula(std::shared_ptr<::hpnmg::ContinuousAtomicProperty> cap) : type(Type::ContinuousAtomicProperty), continuousAtomicProperty(std::move(cap)) {}
    Formula::Formula(std::shared_ptr<::hpnmg::DiscreteAtomicProperty> dap) : type(Type::DiscreteAtomicProperty), discreteAtomicProperty(std::move(dap)) {}
    Formula::Formula(std::shared_ptr<::hpnmg::Conjunction> conj) : type(Type::Conjunction), conjunction(std::move(conj)) {}

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
}
