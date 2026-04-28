#pragma once

#include <fmt/core.h>
#include <string_view>

namespace moth_ui {
    /// @brief Severity levels for log messages.
    enum class LogLevel {
        Debug,   ///< Diagnostic messages, typically disabled in release builds.
        Info,    ///< General informational messages.
        Warning, ///< Recoverable issues that may indicate a problem.
        Error    ///< Unrecoverable failures.
    };

    /**
     * @brief Abstract logger interface.
     *
     * Implementations route messages to a file, console, or external tool.
     * The @c Debug / @c Info / @c Warning / @c Error convenience methods use
     * @c fmt::format for compile-time format-string checking.
     */
    class ILogger {
    public:
        /// @brief Logs a pre-formatted message at the given severity level.
        virtual void Log(LogLevel level, std::string_view message) = 0;

        /// @brief Logs a debug message with @c fmt formatting.
        template <typename... Args>
        void Debug(fmt::format_string<Args...> fmt, Args&&... args) {
            Log(LogLevel::Debug, fmt::format(fmt, std::forward<Args>(args)...));
        }

        /// @brief Logs an info message with @c fmt formatting.
        template <typename... Args>
        void Info(fmt::format_string<Args...> fmt, Args&&... args) {
            Log(LogLevel::Info, fmt::format(fmt, std::forward<Args>(args)...));
        }

        /// @brief Logs a warning message with @c fmt formatting.
        template <typename... Args>
        void Warning(fmt::format_string<Args...> fmt, Args&&... args) {
            Log(LogLevel::Warning, fmt::format(fmt, std::forward<Args>(args)...));
        }

        /// @brief Logs an error message with @c fmt formatting.
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

    /**
     * @brief Logger implementation that silently discards all messages.
     *
     * Used as the default when no logger has been registered via @c SetLogger().
     */
    class NullLogger : public ILogger {
    public:
        /// @brief Discards the message.
        void Log(LogLevel /*level*/, std::string_view /*message*/) override {}
    };

    /// @brief Registers a logger for use by the moth_ui library. Thread-safe.
    ///        Pass @c nullptr to revert to the silent NullLogger.
    /// @note The caller must guarantee the logger outlives all moth_ui usage.
    ///       Call @c SetLogger(nullptr) before destroying the logger.
    void SetLogger(ILogger* logger);

    /// @brief Returns the active logger, or a NullLogger if none has been set. Thread-safe.
    ILogger& GetLogger();

    namespace log {
        template <typename... Args>
        void debug(fmt::format_string<Args...> fmt, Args&&... args) {
            GetLogger().Debug(fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void info(fmt::format_string<Args...> fmt, Args&&... args) {
            GetLogger().Info(fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void warn(fmt::format_string<Args...> fmt, Args&&... args) {
            GetLogger().Warning(fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void error(fmt::format_string<Args...> fmt, Args&&... args) {
            GetLogger().Error(fmt, std::forward<Args>(args)...);
        }
    }
}
