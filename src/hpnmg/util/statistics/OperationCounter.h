#pragma once

#include "flags.h"

#ifdef HPNMG_STATISTICS
#ifndef INCL_FROM_CTR_REPO
	static_assert(false, "This file may only be included indirectly by CounterRepository.h");
#endif

namespace hpnmg {
namespace statistics{

	/**
	 * @brief      Based on github.com/hypro/hypro
	 */
	struct OperationCounter {
		unsigned long val;
		unsigned long operator++() { return ++val; }
		operator unsigned long() const { return val; }
		void reset() {
			val = 0;
		}

		friend std::ostream& operator<<(std::ostream& ostr, const OperationCounter& opCnt) {
			ostr << opCnt.val;
			return ostr;
		}
	};

	struct AtomicOperationCounter : public OperationCounter {
		unsigned long operator++() { return ++val; }
	};

} // namespace statistics
} // namespace hypro

#endif
