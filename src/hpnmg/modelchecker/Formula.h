#ifndef HPNMG_FORMULA_H
#define HPNMG_FORMULA_H

#include <memory>
#include <vector>

#include "modelchecker/DiscreteAtomicProperty.h"

namespace hpnmg {
    class Conjunction;

    class Formula {
    public:
        enum class Type {
            DiscreteAtomicProperty,
            ContinuousAtomicProperty,
            Conjunction,
            Negation,
            Until,
        };

        explicit Formula(std::shared_ptr<::hpnmg::DiscreteAtomicProperty> dap);
        explicit Formula(std::shared_ptr<::hpnmg::Conjunction> conj);

        std::shared_ptr<::hpnmg::Conjunction> getConjunction() const;
        std::shared_ptr<::hpnmg::DiscreteAtomicProperty> getDiscreteAtomicProperty() const;
        Type getType() const;
    private:
        Type type;
        std::shared_ptr<::hpnmg::DiscreteAtomicProperty> discreteAtomicProperty = nullptr;
        std::shared_ptr<::hpnmg::Conjunction> conjunction = nullptr;
    };
}


#endif //HPNMG_FORMULA_H
