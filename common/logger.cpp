#include "logger.hpp"

Logger::Logger(const std::string& path)
    : file(path, std::ios::app),
      mutex(),
      type_msgs{"DEBUG", "SUCCESS", "INFO", "WARNING", "ERROR"} {
    if (!file.is_open())
        throw std::runtime_error("failed to open the log file: " + path);
}

Logger::~Logger() {
    if (file.is_open()) {
        file.flush();
        file.close();
    }
}

void Logger::set_callback(Callback callback) {
    std::lock_guard lock(mutex);
    this->callback = std::move(callback);
}

void Logger::write_logfile(Type type, const std::string& message) {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
    localtime_r(&time, &tm);

    file << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " [" << type_msgs[type]
         << "] -> " << message << "\n";
    file.flush();
}
