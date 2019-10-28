#include "Logging.h"

#ifdef HPNMG_LOGGING

namespace hpnmg {
    void initializeLogging(carl::logging::LogLevel coutLevel, carl::logging::LogLevel fileLevel) {

        if (coutLevel == carl::logging::LogLevel::LVL_DEFAULT) {
            coutLevel = carl::logging::LogLevel::LVL_ERROR;
        }

        if (fileLevel == carl::logging::LogLevel::LVL_DEFAULT) {
            fileLevel = carl::logging::LogLevel::LVL_INFO;
        }

        carl::logging::logger().configure("logfile_hpnmg", "hpnmg.log");
        carl::logging::logger().filter("logfile_hpnmg")
                ("hpnmg", fileLevel);

        carl::logging::logger().configure("stdout", std::cout);
        carl::logging::logger().filter("stdout")
                ("hpnmg", coutLevel);
    }

    carl::logging::LogLevel getLogLevelFromString(const std::string& level) {

        if (level == "ALL") {
            return carl::logging::LogLevel::LVL_ALL;
        } else if (level == "TRACE") {
            return carl::logging::LogLevel::LVL_TRACE;
        } else if (level == "DEBUG") {
            return carl::logging::LogLevel::LVL_DEBUG;
        } else if (level == "INFO") {
            return carl::logging::LogLevel::LVL_INFO;
        } else if (level == "WARN") {
            return carl::logging::LogLevel::LVL_WARN;
        } else if (level == "ERROR") {
            return carl::logging::LogLevel::LVL_ERROR;
        } else if (level == "FATAL") {
            return carl::logging::LogLevel::LVL_FATAL;
        } else {
            return carl::logging::LogLevel::LVL_DEFAULT;
        }
    }

    void initializeLogging(std::string coutLevel, std::string fileLevel) {
        return initializeLogging(
                getLogLevelFromString(coutLevel),
                getLogLevelFromString(fileLevel));
    }

}

#endif