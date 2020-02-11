#pragma once

#include "flags.h"

#ifndef INCL_FROM_STATISTICS
static_assert(false, "This file may only be included indirectly by statistics.h");
#endif

#define INCL_FROM_CTR_REPO

#include "OperationCounter.h"
#include <string>
#include <map>

namespace hpnmg {
    /**
	 * @brief      Based on github.com/hypro/hypro
	 */
    namespace statistics {

        class CounterRepository {
        private:
            using CounterMapType = std::map<std::string, OperationCounter*>;

            CounterMapType counterMap;

        public:
            void add(std::string name) {
                if(counterMap.find(name) == counterMap.end()) {
                    counterMap[name] = new OperationCounter();
                }
            }

            void reset() {
                counterMap.clear();
            }

            OperationCounter& get(std::string name) {
                auto counterIt = counterMap.find(name);
                if( counterIt == counterMap.end() ) {
                    counterIt = counterMap.insert(std::make_pair(name, new OperationCounter())).first;
                }
                return *(counterIt->second);
            }

            std::size_t size() const {
                return counterMap.size();
            }

            friend std::ostream& operator<<(std::ostream& ostr, const CounterRepository& repo) {
                for(const auto& counterPair : repo.counterMap) {
                    ostr << *counterPair.second << "\t" << counterPair.first << std::endl;
                }
                return ostr;
            }
        };

    } // namespace statistics
} // namespace hpnmg

//#endif
