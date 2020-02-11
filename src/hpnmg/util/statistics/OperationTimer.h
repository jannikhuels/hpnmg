#pragma once

#include "flags.h"

#ifdef HPNMG_STATISTICS
#ifndef INCL_FROM_CTR_REPO
	static_assert(false, "This file may only be included indirectly by TimerRepository.h");
#endif

#include <stdexcept>
#include <iomanip>

namespace hpnmg {
namespace statistics{

	/**
	 * @brief      Based on github.com/hypro/hypro
	 */
	struct OperationTimer {
		std::unique_ptr<std::chrono::high_resolution_clock::time_point> startTime = nullptr;
		std::chrono::duration<double> duration = std::chrono::duration<double>::zero();

		void start() {
		    if (startTime != nullptr) {
		        throw std::logic_error("Timer already started.");
		    }
		    startTime = std::make_unique<std::chrono::high_resolution_clock::time_point>(std::chrono::high_resolution_clock::now());
		}

		void stop() {
            if (startTime == nullptr) {
                throw std::logic_error("Timer not started.");
            }
            duration += std::chrono::high_resolution_clock::now() - *startTime;
            startTime = nullptr;
		}

		operator double() const { return duration.count(); }

		void reset() {
			startTime = nullptr;
			duration = std::chrono::duration<double>::zero();;
		}

		friend std::ostream& operator<<(std::ostream& ostr, const OperationTimer& opTimer) {
			ostr << std::fixed << std::setprecision(4) << opTimer.duration.count();
			return ostr;
		}
	};

} // namespace statistics
} // namespace hypro

#endif
