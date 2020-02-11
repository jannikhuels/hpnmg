#pragma once

#include "flags.h"

#ifndef INCL_FROM_STATISTICS
static_assert(false, "This file may only be included indirectly by Statistics.h");
#endif

#define INCL_FROM_CTR_REPO

#include <string>
#include <map>
#include "OperationTimer.h"

namespace hpnmg {
    /**
	 * @brief      Based on github.com/hypro/hypro
	 */
    namespace statistics {

        class TimerRepository {
        private:
            using TimerMapType = std::map<std::string, OperationTimer*>;

            TimerMapType timerMap;

        public:
            void start(std::string name) {
                auto timerIt = timerMap.find(name);
                if(timerIt == timerMap.end()) {
                    timerMap[name] = new OperationTimer();
                    timerMap[name]->start();
                    return;
                }
                timerIt->second->start();
            }

            void reset() {
                timerMap.clear();
            }

            void stop(std::string name) {
                auto timerIt = timerMap.find(name);
                if(timerIt == timerMap.end() ) {
                    return;
                }
                timerIt->second->stop();
            }

            std::size_t size() const {
                return timerMap.size();
            }

            friend std::ostream& operator<<(std::ostream& ostr, const TimerRepository& repo) {
                for(const auto& timerPair : repo.timerMap) {
                    ostr << *timerPair.second << "\t" << timerPair.first << std::endl;
                }
                return ostr;
            }

        };

    } // namespace statistics
} // namespace hpnmg

//#endif
