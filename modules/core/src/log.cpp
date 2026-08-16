#include "moth/core/log.h"

#include <atomic>

namespace moth::core {
    namespace {
        std::atomic<ILogger*> s_logger{ nullptr };
        NullLogger s_nullLogger;
    }

    void SetLogger(ILogger* logger) {
        s_logger.store(logger, std::memory_order_release);
    }

    ILogger& GetLogger() {
        ILogger* logger = s_logger.load(std::memory_order_acquire);
        if (logger != nullptr) {
            return *logger;
        }
        return s_nullLogger;
    }
}
