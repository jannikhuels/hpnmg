#ifndef HPNMG_FORMULA_H
#define HPNMG_FORMULA_H

#include <memory>
#include <vector>

#include "modelchecker/ContinuousAtomicProperty.h"
#include "modelchecker/DiscreteAtomicProperty.h"

namespace hpnmg {
    class Conjunction;
    class Negation;
    class Until;

    class Formula {
    public:
        enum class Type {
            Conjunction,
            ContinuousAtomicProperty,
            DiscreteAtomicProperty,
            Negation,
            Until,
        };

        explicit Formula(std::shared_ptr<::hpnmg::ContinuousAtomicProperty> cap);
        explicit Formula(std::shared_ptr<::hpnmg::DiscreteAtomicProperty> dap);
        explicit Formula(std::shared_ptr<::hpnmg::Conjunction> conj);
        explicit Formula(std::shared_ptr<::hpnmg::Negation> neg);
        explicit Formula(std::shared_ptr<::hpnmg::Until> until);

        std::shared_ptr<::hpnmg::Conjunction> getConjunction() const;
        std::shared_ptr<::hpnmg::ContinuousAtomicProperty> getContinuousAtomicProperty() const;
        std::shared_ptr<::hpnmg::DiscreteAtomicProperty> getDiscreteAtomicProperty() const;
        std::shared_ptr<::hpnmg::Negation> getNegation() const;
        std::shared_ptr<::hpnmg::Until> getUntil() const;
        Type getType() const;
    private:
        Type type;
        std::shared_ptr<::hpnmg::Conjunction> conjunction = nullptr;
        std::shared_ptr<::hpnmg::ContinuousAtomicProperty> continuousAtomicProperty = nullptr;
        std::shared_ptr<::hpnmg::DiscreteAtomicProperty> discreteAtomicProperty = nullptr;
        std::shared_ptr<::hpnmg::Negation> negation = nullptr;
        std::shared_ptr<::hpnmg::Until> until = nullptr;
    };
}


#endif //HPNMG_FORMULA_H
