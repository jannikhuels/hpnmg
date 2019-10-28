#pragma once

#define HPNMG_LOGGING

#define TRACELOG(channel,expr)
#define DEBUGLOG(channel,expr)
#define INFOLOG(channel,expr)
#define WARNLOG(channel,expr)
#define ERRORLOG(channel,expr)
#define FATALLOG(channel,expr)

#ifdef HPNMG_LOGGING

#include <carl/core/carlLogging.h>

namespace hpnmg{
    void initializeLogging(carl::logging::LogLevel coutLevel, carl::logging::LogLevel fileLevel);
    void initializeLogging(std::string coutLevel, std::string fileLevel);

}

#undef TRACELOG
#undef DEBUGLOG
#undef INFOLOG
#undef WARNLOG
#undef ERRORLOG
#undef FATALLOG

#define __HPNMG_LOG(lvl,channel,expr) {std::stringstream __ss; __ss << expr; carl::logging::Logger::getInstance().log(lvl, channel, __ss, carl::logging::RecordInfo{__FILE__,__func__,__LINE__});}

#define TRACELOG(channel,expr) __HPNMG_LOG(carl::logging::LogLevel::LVL_TRACE, channel, expr)
#define DEBUGLOG(channel,expr) __HPNMG_LOG(carl::logging::LogLevel::LVL_DEBUG, channel, expr)
#define INFOLOG(channel,expr) __HPNMG_LOG(carl::logging::LogLevel::LVL_INFO, channel, expr)
#define WARNLOG(channel,expr) __HPNMG_LOG(carl::logging::LogLevel::LVL_WARN, channel, expr)
#define ERRORLOG(channel,expr) __HPNMG_LOG(carl::logging::LogLevel::LVL_ERROR, channel, expr)
#define FATALLOG(channel,expr) __HPNMG_LOG(carl::logging::LogLevel::LVL_FATAL, channel, expr)

#endif
