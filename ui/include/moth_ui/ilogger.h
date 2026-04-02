#pragma once

#include <fmt/core.h>
#include <string_view>

namespace moth_ui {
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    class ILogger {
    public:
        virtual void Log(LogLevel level, std::string_view message) = 0;

        template <typename... Args>
        void Debug(fmt::format_string<Args...> fmt, Args&&... args) {
            Log(LogLevel::Debug, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void Info(fmt::format_string<Args...> fmt, Args&&... args) {
            Log(LogLevel::Info, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void Warning(fmt::format_string<Args...> fmt, Args&&... args) {
            Log(LogLevel::Warning, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void Error(fmt::format_string<Args...> fmt, Args&&... args) {
            Log(LogLevel::Error, fmt::format(fmt, std::forward<Args>(args)...));
        }

        ILogger() = default;
        ILogger(ILogger const&) = default;
        ILogger(ILogger&&) = default;
        ILogger& operator=(ILogger const&) = default;
        ILogger& operator=(ILogger&&) = default;
        virtual ~ILogger() = default;
    };

    class NullLogger : public ILogger {
    public:
        void Log(LogLevel /*level*/, std::string_view /*message*/) override {}
    };

    /// @brief Registers a logger for use by the moth_ui library.
    ///        Pass @c nullptr to revert to the silent NullLogger.
    void SetLogger(ILogger* logger);

    /// @brief Returns the active logger, or a NullLogger if none has been set.
    ILogger& GetLogger();
}
