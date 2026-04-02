#include "moth_ui/ilogger.h"

namespace moth_ui {
    namespace {
        ILogger* s_logger = nullptr;
        NullLogger s_nullLogger;
    }

    void SetLogger(ILogger* logger) {
        s_logger = logger;
    }

    ILogger& GetLogger() {
        if (s_logger != nullptr) {
            return *s_logger;
        }
        return s_nullLogger;
    }
}
