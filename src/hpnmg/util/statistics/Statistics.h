#pragma once

#include "flags.h"
#include <carl/util/Singleton.h>

#define PRINT_STATS()
#define RESET_STATS()
#define COUNT_STATS(expr)
#define START_BENCHMARK_OPERATION(expr)
#define STOP_BENCHMARK_OPERATION(expr)
#define RESET_BENCHMARK()

#ifdef HPNMG_STATISTICS
#define INCL_FROM_STATISTICS

#include "CounterRepository.h"
#include "TimerRepository.h"
#include "util/logging/Logging.h"

namespace hpnmg {
namespace statistics {

    /**
	 * @brief      Based on github.com/hypro/hypro
	 */
	class Statistician : public carl::Singleton<Statistician> {
		friend carl::Singleton<Statistician>;

	private:
		CounterRepository counters;
		TimerRepository timers;

	protected:
        Statistician() {
            carl::logging::logger().configure("statistics_hpnmg", "hpnmg-statistics.log");
            carl::logging::logger().filter("statistics_hpnmg")
                    ("statistics", carl::logging::LogLevel::LVL_INFO)
                    ("hypro", carl::logging::LogLevel::LVL_OFF);
            auto formatter = carl::logging::logger().formatter("statistics_hpnmg");
            formatter->printInformation = false;
            carl::logging::logger().formatter("statistics_hpnmg", formatter);
        }

	public:
		~Statistician(){}

		void addCounter(std::string name) {
			counters.add(name);
		}

		OperationCounter& getCounter(std::string name) {
			return counters.get(name);
		}

		void resetCounter() {
			counters.reset();
		}

		friend std::ostream& operator<<(std::ostream& ostr, const Statistician& stats) {
			ostr << "Statistics" << std::endl;
            ostr << "\t==-== \t Timers \t ==-==" << std::endl;
			ostr << stats.timers;
            ostr << "\t==-== \t Counters \t ==-==" << std::endl;
			ostr << stats.counters;
			return ostr;
		}

		void startTimer(std::string name) {
            timers.start(name);
        }

        void stopTimer(std::string name) {
            timers.stop(name);
        }

        void resetTimer() {
            timers.reset();
        }
	};

} // namespace statistics
} // namespace hpnmg

#undef PRINT_STATS
#undef RESET_STATS
#undef COUNT_STATS
#undef START_BENCHMARK_OPERATION
#undef STOP_BENCHMARK_OPERATION
#undef RESET_BENCHMARK

#define __HPNMG_WRITE_STATISTICS(channel,expr) {std::stringstream __ss; __ss << expr; carl::logging::Logger::getInstance().log(carl::logging::LogLevel::LVL_INFO, channel, __ss, carl::logging::RecordInfo{__FILE__,__func__,__LINE__});}

#define PRINT_STATS() __HPNMG_WRITE_STATISTICS("statistics",hpnmg::statistics::Statistician::getInstance());
#define RESET_STATS() hpnmg::statistics::Statistician::getInstance().resetCounter();
#define COUNT_STATS(expr) ++hpnmg::statistics::Statistician::getInstance().getCounter(expr);
#define START_BENCHMARK_OPERATION(name) hpnmg::statistics::Statistician::getInstance().startTimer(name);
#define STOP_BENCHMARK_OPERATION(name) hpnmg::statistics::Statistician::getInstance().stopTimer(name);
#define RESET_BENCHMARK() hpnmg::statistics::Statistician::getInstance()::resetTimer();

#endif
