#ifndef HPNMG_FORMULA_H
#define HPNMG_FORMULA_H

#include "modelchecker/ContinuousAtomicProperty.h"
#include "modelchecker/DiscreteAtomicProperty.h"
#include "modelchecker/False.h"
#include "modelchecker/True.h"

#include <memory>
#include <vector>

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
            False,
            Negation,
            True,
            Until,
        };

        explicit Formula(std::shared_ptr<::hpnmg::Conjunction> conj);
        explicit Formula(std::shared_ptr<::hpnmg::ContinuousAtomicProperty> cap);
        explicit Formula(std::shared_ptr<::hpnmg::DiscreteAtomicProperty> dap);
        explicit Formula(std::shared_ptr<::hpnmg::False>);
        explicit Formula(std::shared_ptr<::hpnmg::Negation> neg);
        explicit Formula(std::shared_ptr<::hpnmg::True>);
        explicit Formula(std::shared_ptr<::hpnmg::Until> until);

        std::shared_ptr<::hpnmg::Conjunction> getConjunction() const;
        std::shared_ptr<::hpnmg::ContinuousAtomicProperty> getContinuousAtomicProperty() const;
        std::shared_ptr<::hpnmg::DiscreteAtomicProperty> getDiscreteAtomicProperty() const;
        std::shared_ptr<::hpnmg::Negation> getNegation() const;
        std::shared_ptr<::hpnmg::Until> getUntil() const;
        Type getType() const;

        /**
         * Determine if the formula is either always true or always false.
         *
         * Example formulae:
         * 1. "True" is trivially true.
         * 2. "False" is trivially false.
         * 3. "True And False" is trivially false
         * 4. "Psi Until Phi" is trivial if and only if Phi is trivial.
         * 5. "Not True" is trivially false.
         * 6. "m(P1) = c" is not trivial.
         * 7. "x(P1) <= c" is not trivial.
         *
         * @return If <code>result.first</code> is true, then the Formula is trivial and <code>result.second</code> is
         *         a True-formula or False-formula instance representing this formula. If <code>result.first</code> is
         *         false, <code>result.second</code> is unspecified.
         *         TODO: replace std::pair with std::optional<Formula> in c++17
         */
        std::pair<bool, Formula> isTrivial() const;
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
