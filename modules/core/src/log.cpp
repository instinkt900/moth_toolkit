#include "moth/core/log.h"

#include <spdlog/spdlog.h>

#include <atomic>

namespace moth::core {
    namespace {
        std::atomic<ILogger*> s_logger{ nullptr };

        // Default logger: routes to spdlog's console sink, so moth::core::log::*
        // prints out of the box without a host registering a logger.
        class SpdlogLogger : public ILogger {
        public:
            void Log(LogLevel level, std::string_view message) override {
                switch (level) {
                case LogLevel::Debug:   spdlog::debug("{}", message); break;
                case LogLevel::Info:    spdlog::info("{}", message); break;
                case LogLevel::Warning: spdlog::warn("{}", message); break;
                case LogLevel::Error:   spdlog::error("{}", message); break;
                }
            }
        };

        SpdlogLogger s_defaultLogger;
    }

    void SetLogger(ILogger* logger) {
        s_logger.store(logger, std::memory_order_release);
    }

    ILogger& GetLogger() {
        ILogger* logger = s_logger.load(std::memory_order_acquire);
        if (logger != nullptr) {
            return *logger;
        }
        return s_defaultLogger;
    }
}
