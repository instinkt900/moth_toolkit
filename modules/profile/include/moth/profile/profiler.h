#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

namespace moth::profile {
    /**
     * @brief One timed scope within a recorded frame.
     */
    struct ScopeRecord {
        char const* name = nullptr;             ///< Scope name; must outlive the profiler (a string literal).
        int depth = 0;                          ///< Nesting depth: 0 for a scope opened directly in the frame.
        std::chrono::nanoseconds start{ 0 };    ///< Start time, relative to the start of the frame.
        std::chrono::nanoseconds duration{ 0 }; ///< Time spent in the scope, including its nested scopes.
    };

    /**
     * @brief A recorded frame: its duration and every scope timed within it.
     *
     * @c scopes are in the order they were opened, so a scope's nested scopes
     * follow it directly with a greater @c depth.
     */
    struct FrameRecord {
        std::uint64_t number = 0;               ///< Frame number, counting recorded frames from 1.
        std::chrono::nanoseconds duration{ 0 }; ///< Time from this frame's mark to the next.
        std::vector<ScopeRecord> scopes;
    };

    /**
     * @brief Records nested scope timings per frame and keeps a history of recent frames.
     *
     * Call @c MarkFrame once per loop iteration: it completes the frame in
     * progress and starts the next, so a frame's duration covers everything
     * between two marks, including time no scope covers. Time scopes with
     * @c ProfileScope or @c MOTH_PROFILE_SCOPE.
     *
     * Only the thread that first calls @c MarkFrame is recorded; marks and scopes
     * from other threads are ignored.
     */
    class Profiler {
    public:
        static constexpr std::size_t kDefaultHistorySize = 300;

        /// @brief Constructs a profiler that keeps the last @p historySize completed frames (at least 1).
        explicit Profiler(std::size_t historySize = kDefaultHistorySize);

        /// @brief Returns the process-wide profiler used by the @c MOTH_PROFILE_* macros.
        static Profiler& Get();

        /// @brief Completes the frame in progress, adding it to the history, and starts the next one.
        ///
        /// Scopes still open at the mark are closed at the mark; their later ends are ignored.
        void MarkFrame();

        /// @brief Opens a scope named @p name in the current frame.
        ///
        /// @p name must be non-null and outlive the profiler. Returns a token for
        /// @c EndScope, or 0 if the scope is not recorded (no frame in progress,
        /// recording paused, or called from another thread).
        std::uint64_t BeginScope(char const* name);

        /// @brief Closes the most recently opened scope, if @p token is from the current frame.
        void EndScope(std::uint64_t token);

        /// @brief Pauses or resumes recording.
        ///
        /// Takes effect at the next @c MarkFrame, so the frame in progress still completes.
        void SetPaused(bool paused) { m_paused = paused; }

        /// @brief Returns @c true if recording is paused.
        bool IsPaused() const { return m_paused; }

        /// @brief Returns the number of completed frames in the history.
        std::size_t GetFrameCount() const { return m_frameCount; }

        /// @brief Returns completed frame @p index, where 0 is the oldest. @p index must be below @c GetFrameCount().
        FrameRecord const& GetFrame(std::size_t index) const;

    private:
        using Clock = std::chrono::steady_clock;

        std::vector<FrameRecord> m_history; // ring buffer of completed frames
        std::size_t m_nextSlot = 0;
        std::size_t m_frameCount = 0;

        FrameRecord m_current;
        Clock::time_point m_frameStart;
        std::vector<std::size_t> m_openScopes; // indices into m_current.scopes, innermost last
        std::uint64_t m_lastFrameNumber = 0;
        bool m_inFrame = false;
        bool m_paused = false;
        std::thread::id m_threadId;
    };

    /**
     * @brief Times the enclosing block as a scope of the current frame.
     */
    class ProfileScope {
    public:
        explicit ProfileScope(char const* name, Profiler& profiler = Profiler::Get())
            : m_profiler(profiler)
            , m_token(profiler.BeginScope(name)) {}

        ~ProfileScope() { m_profiler.EndScope(m_token); }

        ProfileScope(ProfileScope const&) = delete;
        ProfileScope& operator=(ProfileScope const&) = delete;
        ProfileScope(ProfileScope&&) = delete;
        ProfileScope& operator=(ProfileScope&&) = delete;

    private:
        Profiler& m_profiler;
        std::uint64_t m_token;
    };
}

#define MOTH_PROFILE_CONCAT_IMPL(a, b) a##b
#define MOTH_PROFILE_CONCAT(a, b) MOTH_PROFILE_CONCAT_IMPL(a, b)

// Define MOTH_PROFILE_DISABLE to compile the macros out.
#ifndef MOTH_PROFILE_DISABLE
/// @brief Times the rest of the enclosing block as a scope named @p name (a string literal).
#define MOTH_PROFILE_SCOPE(name) ::moth::profile::ProfileScope MOTH_PROFILE_CONCAT(mothProfileScope, __LINE__)(name)
/// @brief Marks the boundary between two frames on the process-wide profiler.
#define MOTH_PROFILE_FRAME() ::moth::profile::Profiler::Get().MarkFrame()
#else
#define MOTH_PROFILE_SCOPE(name) static_cast<void>(0)
#define MOTH_PROFILE_FRAME() static_cast<void>(0)
#endif
