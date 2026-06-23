#include "logger.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace fml {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    flush();
    if (logFile_.is_open()) {
        logFile_.close();
    }
}

void Logger::init(const std::string& logDir) {
    std::lock_guard<std::mutex> lock(mutex_);
    logDir_ = logDir;
    std::filesystem::create_directories(logDir);

    currentLogFile_ = logDir_ + "/fml.log";
    logFile_.open(currentLogFile_, std::ios::app);
    if (logFile_.is_open()) {
        currentLogSize_ = static_cast<uint64_t>(logFile_.tellp());
    }
}

void Logger::log(LogLevel level, const std::string& module, const std::string& message) {
    if (level < currentLevel_) return;

    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream oss;
    oss << "{";
    oss << "\"timestamp\":\"" << timestamp() << "\",";
    oss << "\"level\":\"" << levelToString(level) << "\",";
    oss << "\"module\":\"" << module << "\",";
    oss << "\"message\":\"" << message << "\"";
    oss << "}" << std::endl;

    std::string logLine = oss.str();

    rotateIfNeeded();
    if (logFile_.is_open()) {
        logFile_ << logLine;
        currentLogSize_ += logLine.size();
    }

    if (callback_) {
        callback_(level, module, message);
    }
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    currentLevel_ = level;
}

void Logger::setCallback(LogCallback cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = std::move(cb);
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        logFile_.flush();
    }
}

void Logger::rotateIfNeeded() {
    if (currentLogSize_ < MAX_LOG_SIZE) return;

    if (logFile_.is_open()) {
        logFile_.close();
    }

    for (uint32_t i = MAX_LOG_FILES - 1; i > 0; --i) {
        std::string oldFile = logDir_ + "/fml.log." + std::to_string(i);
        std::string newFile = logDir_ + "/fml.log." + std::to_string(i + 1);
        if (std::filesystem::exists(oldFile)) {
            if (i + 1 >= MAX_LOG_FILES) {
                std::filesystem::remove(oldFile);
            } else {
                std::filesystem::rename(oldFile, newFile);
            }
        }
    }

    std::string rotatedFile = logDir_ + "/fml.log.1";
    if (std::filesystem::exists(currentLogFile_)) {
        std::filesystem::rename(currentLogFile_, rotatedFile);
    }

    logFile_.open(currentLogFile_, std::ios::app);
    currentLogSize_ = 0;
    currentLogIndex_++;
}

std::string Logger::timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default:              return "UNKNOWN";
    }
}

} // namespace fml