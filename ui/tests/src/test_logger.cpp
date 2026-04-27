#include "moth_ui/ilogger.h"
#include <catch2/catch_all.hpp>

using namespace moth_ui;

TEST_CASE("NullLogger does not throw on any log level", "[logger][null]") {
    NullLogger logger;
    REQUIRE_NOTHROW(logger.Log(LogLevel::Debug, "test"));
    REQUIRE_NOTHROW(logger.Log(LogLevel::Info, "test"));
    REQUIRE_NOTHROW(logger.Log(LogLevel::Warning, "test"));
    REQUIRE_NOTHROW(logger.Log(LogLevel::Error, "test"));
}

TEST_CASE("NullLogger formatted methods do not throw", "[logger][null]") {
    NullLogger logger;
    REQUIRE_NOTHROW(logger.Debug("debug message {}", 42));
    REQUIRE_NOTHROW(logger.Info("info message {}", 3.14f));
    REQUIRE_NOTHROW(logger.Warning("warning message {}", "text"));
    REQUIRE_NOTHROW(logger.Error("error message {}", true));
}

TEST_CASE("GetLogger returns valid reference without explicit SetLogger", "[logger][global]") {
    ILogger& logger = GetLogger();
    REQUIRE_NOTHROW(logger.Log(LogLevel::Info, "test"));
}

TEST_CASE("SetLogger with NullLogger does not throw", "[logger][global]") {
    NullLogger null;
    REQUIRE_NOTHROW(SetLogger(&null));
    REQUIRE_NOTHROW(SetLogger(nullptr));
}
