#include "STDPolytope.h"
#include "STDPolytope.tpp"

#include <gmpxx.h>

// If we want users of the library to be able to choose the <code>Numeric</code> type for STDPolytpe, the template
// definitions *need* to be in the STDPolytope header.
// However, this will greatly increase compilation time during development of hpnmg because for every change to the
// implementation, i.e. the template definitions, every file including the header needs to be recompiled.
// Instead, the template file is commented out in the header and the template is explicitly instantiated here for the
// types we actually use during development.
template class hpnmg::STDPolytope<double>;
template class hpnmg::STDPolytope<mpq_class>;
