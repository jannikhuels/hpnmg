#include "STDPolytope.h"
#include "STDPolytope.tpp"
#include "util/logging/Logging.h"

#include <gmpxx.h>

namespace hpnmg {
    hypro::HPolytope<mpq_class> convertHPolytope(const hypro::HPolytope<double>& from) {
        const auto& fromConstraints = from.constraints();
        auto toConstraints = std::vector<hypro::Halfspace<mpq_class>>();
        toConstraints.reserve(fromConstraints.size());

        std::transform(fromConstraints.begin(), fromConstraints.end(), std::back_inserter(toConstraints), [](hypro::Halfspace<double> hsp) {
            const hypro::vector_t<double>& oldNormal = hsp.normal();
            hypro::vector_t<mpq_class> newNormal(oldNormal.size());
            for (int i = 0; i < oldNormal.size(); ++i)
                newNormal[i] = oldNormal[i];

            return hypro::Halfspace<mpq_class>(newNormal, hsp.offset());
        });

        return hypro::HPolytope<mpq_class>(toConstraints);
    }

    hypro::HPolytope<double> convertHPolytope(const hypro::HPolytope<mpq_class>& from) {
        const auto& fromConstraints = from.constraints();
        auto toConstraints = std::vector<hypro::Halfspace<double>>();
        toConstraints.reserve(fromConstraints.size());

        std::transform(fromConstraints.begin(), fromConstraints.end(), std::back_inserter(toConstraints), [](hypro::Halfspace<mpq_class> hsp) {
            const auto& oldNormal = hsp.normal();
            auto newNormal = hypro::vector_t<double>(oldNormal.size());
            for (int i = 0; i < oldNormal.size(); ++i)
                newNormal[i] = oldNormal[i].get_d();

            return hypro::Halfspace<double>(newNormal, hsp.offset().get_d());
        });

        return hypro::HPolytope<double>(toConstraints);
    }
}

// If we want users of the library to be able to choose the <code>Numeric</code> type for STDPolytpe, the template
// definitions *need* to be in the STDPolytope header.
// However, this will greatly increase compilation time during development of hpnmg because for every change to the
// implementation, i.e. the template definitions, every file including the header needs to be recompiled.
// Instead, the template file is commented out in the header and the template is explicitly instantiated here for the
// types we actually use during development.
template class hpnmg::STDPolytope<double>;
template class hpnmg::STDPolytope<mpq_class>;

template hpnmg::STDPolytope<double>::operator STDPolytope<mpq_class>() const;
template hpnmg::STDPolytope<mpq_class>::operator STDPolytope<double>() const;
template std::ostream& operator<<(std::ostream &os, const hpnmg::STDPolytope<mpq_class> &region);
template std::ostream& operator<<(std::ostream &os, const hpnmg::STDPolytope<double> &region);
