#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <functional>
#include <filesystem>
#include <cstdint>

#include "fivemlinux/types.h"

namespace fml {

class Logger {
public:
    static Logger& instance();

    void init(const std::string& logDir);
    void log(LogLevel level, const std::string& module, const std::string& message);
    void setLevel(LogLevel level);
    void setCallback(LogCallback cb);
    void flush();

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void rotateIfNeeded();
    std::string timestamp() const;
    std::string levelToString(LogLevel level) const;

    std::ofstream logFile_;
    std::mutex mutex_;
    LogLevel currentLevel_ = LogLevel::Info;
    LogCallback callback_;
    std::string logDir_;
    std::string currentLogFile_;
    uint32_t currentLogIndex_ = 0;
    uint64_t currentLogSize_ = 0;
    static constexpr uint64_t MAX_LOG_SIZE = 10 * 1024 * 1024; // 10MB
    static constexpr uint32_t MAX_LOG_FILES = 5;
};

#define FML_LOG(level, module, msg) \
    fml::Logger::instance().log(level, module, msg)

} // namespace fml