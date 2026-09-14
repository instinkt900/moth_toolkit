#include "moth/profile/profiler.h"

#include <catch2/catch_all.hpp>

#include <string>
#include <thread>

using namespace moth::profile;

TEST_CASE("Profiler: scopes outside a frame are not recorded", "[profile]") {
    Profiler profiler;
    {
        ProfileScope scope("before the first mark", profiler);
    }
    profiler.MarkFrame();
    profiler.MarkFrame();

    REQUIRE(profiler.GetFrameCount() == 1);
    REQUIRE(profiler.GetFrame(0).number == 1);
    REQUIRE(profiler.GetFrame(0).scopes.empty());
}

TEST_CASE("Profiler: nested scopes record depth, order, and containment", "[profile]") {
    Profiler profiler;
    profiler.MarkFrame();
    {
        ProfileScope update("update", profiler);
        {
            ProfileScope a("a", profiler);
        }
        {
            ProfileScope b("b", profiler);
            ProfileScope c("c", profiler);
        }
    }
    {
        ProfileScope draw("draw", profiler);
    }
    profiler.MarkFrame();

    REQUIRE(profiler.GetFrameCount() == 1);
    auto const& frame = profiler.GetFrame(0);
    REQUIRE(frame.scopes.size() == 5);

    REQUIRE(std::string(frame.scopes[0].name) == "update");
    REQUIRE(frame.scopes[0].depth == 0);
    REQUIRE(std::string(frame.scopes[1].name) == "a");
    REQUIRE(frame.scopes[1].depth == 1);
    REQUIRE(std::string(frame.scopes[2].name) == "b");
    REQUIRE(frame.scopes[2].depth == 1);
    REQUIRE(std::string(frame.scopes[3].name) == "c");
    REQUIRE(frame.scopes[3].depth == 2);
    REQUIRE(std::string(frame.scopes[4].name) == "draw");
    REQUIRE(frame.scopes[4].depth == 0);

    // A scope contains its children, siblings don't overlap, and the frame contains every scope.
    REQUIRE(frame.scopes[0].duration >= frame.scopes[1].duration + frame.scopes[2].duration);
    REQUIRE(frame.scopes[2].duration >= frame.scopes[3].duration);
    REQUIRE(frame.scopes[4].start >= frame.scopes[0].start + frame.scopes[0].duration);
    REQUIRE(frame.duration >= frame.scopes[4].start + frame.scopes[4].duration);
}

TEST_CASE("Profiler: history keeps the most recent frames, oldest first", "[profile]") {
    Profiler profiler(3);
    for (int i = 0; i < 6; ++i) {
        profiler.MarkFrame(); // the first mark starts frame 1; the rest complete frames 1-5
    }

    REQUIRE(profiler.GetFrameCount() == 3);
    REQUIRE(profiler.GetFrame(0).number == 3);
    REQUIRE(profiler.GetFrame(1).number == 4);
    REQUIRE(profiler.GetFrame(2).number == 5);
}

TEST_CASE("Profiler: pausing takes effect at the next frame mark", "[profile]") {
    Profiler profiler;
    profiler.MarkFrame();
    profiler.SetPaused(true);
    {
        ProfileScope scope("still recorded", profiler); // frame 1 is still in progress
    }
    profiler.MarkFrame(); // completes frame 1 and starts nothing
    {
        ProfileScope scope("ignored", profiler);
    }
    profiler.MarkFrame();

    REQUIRE(profiler.IsPaused());
    REQUIRE(profiler.GetFrameCount() == 1);
    REQUIRE(profiler.GetFrame(0).scopes.size() == 1);
    REQUIRE(std::string(profiler.GetFrame(0).scopes[0].name) == "still recorded");

    profiler.SetPaused(false);
    profiler.MarkFrame(); // starts frame 2
    profiler.MarkFrame(); // completes it

    REQUIRE(profiler.GetFrameCount() == 2);
    REQUIRE(profiler.GetFrame(1).number == 2);
}

TEST_CASE("Profiler: a scope open at a frame mark is closed there and its end ignored", "[profile]") {
    Profiler profiler;
    profiler.MarkFrame();
    {
        ProfileScope outer("outer", profiler);
        profiler.MarkFrame(); // completes frame 1, cutting outer off at the mark
        ProfileScope inner("inner", profiler);
    } // inner closes in frame 2; outer's end belongs to frame 1 and is ignored
    profiler.MarkFrame();

    REQUIRE(profiler.GetFrameCount() == 2);

    auto const& first = profiler.GetFrame(0);
    REQUIRE(first.scopes.size() == 1);
    REQUIRE(std::string(first.scopes[0].name) == "outer");
    REQUIRE(first.scopes[0].start + first.scopes[0].duration == first.duration);

    auto const& second = profiler.GetFrame(1);
    REQUIRE(second.scopes.size() == 1);
    REQUIRE(std::string(second.scopes[0].name) == "inner");
    REQUIRE(second.scopes[0].depth == 0);
}

TEST_CASE("Profiler: marks and scopes from other threads are ignored", "[profile]") {
    Profiler profiler;
    profiler.MarkFrame();
    std::thread([&profiler] {
        profiler.MarkFrame();
        ProfileScope scope("worker", profiler);
    }).join();
    profiler.MarkFrame();

    REQUIRE(profiler.GetFrameCount() == 1);
    REQUIRE(profiler.GetFrame(0).scopes.empty());
}
