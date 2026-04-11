// Pins the layout system: LayoutEntityType enum, LayoutRect, Layout static API,
// LayoutCache, and LayoutEntityFlipbook.

#include "moth_ui/moth_ui.h"

#include <catch2/catch_all.hpp>
#include <type_traits>

using namespace moth_ui;

TEST_CASE("LayoutEntityType enum values are stable", "[api][layout][type]") {
    using T = LayoutEntityType;
    static_assert(T::Unknown  != T::Layout);
    static_assert(T::Layout   != T::Group);
    static_assert(T::Group    != T::Ref);
    static_assert(T::Ref      != T::Entity);
    static_assert(T::Entity   != T::Rect);
    static_assert(T::Rect     != T::Image);
    static_assert(T::Image    != T::Text);
    static_assert(T::Text     != T::Clip);
    static_assert(T::Clip     != T::Flipbook);
    SUCCEED();
}

TEST_CASE("LayoutRect fields are stable", "[api][layout][rect]") {
    static_assert(std::is_same_v<decltype(LayoutRect::anchor), FloatRect>);
    static_assert(std::is_same_v<decltype(LayoutRect::offset), FloatRect>);
    SUCCEED();
}

TEST_CASE("Layout static constants and Load/Save signatures are stable", "[api][layout][layout]") {
    // Static string constants
    static_assert(std::is_same_v<decltype(Layout::Extension),           std::string const>);
    static_assert(std::is_same_v<decltype(Layout::FullExtension),       std::string const>);
    static_assert(std::is_same_v<decltype(Layout::BinaryExtension),     std::string const>);
    static_assert(std::is_same_v<decltype(Layout::FullBinaryExtension), std::string const>);
    static_assert(std::is_same_v<decltype(Layout::Version),             int const>);

    // Load: two overloads, both static
    Layout::LoadResult (*load1)(std::filesystem::path const&,
                                std::shared_ptr<Layout>*) = &Layout::Load;
    Layout::LoadResult (*load2)(std::filesystem::path const&,
                                Layout::LoadOptions const&,
                                std::shared_ptr<Layout>*) = &Layout::Load;
    // Save: two overloads, both const member
    bool (Layout::*save1)(std::filesystem::path const&) const = &Layout::Save;
    bool (Layout::*save2)(std::filesystem::path const&,
                          Layout::SaveOptions const&) const   = &Layout::Save;

    (void)load1; (void)load2; (void)save1; (void)save2;
    SUCCEED();
}

TEST_CASE("LayoutCache method signatures are stable", "[api][layout][cache]") {
    void (LayoutCache::*setRoot)(std::string_view)                   = &LayoutCache::SetLayoutRoot;
    std::shared_ptr<Layout> (LayoutCache::*get)(std::string_view)    = &LayoutCache::GetLayout;
    void (LayoutCache::*flush)()                                     = &LayoutCache::FlushCache;
    (void)setRoot; (void)get; (void)flush;
    SUCCEED();
}

TEST_CASE("LayoutEntityFlipbook field is stable", "[api][layout][flipbook]") {
    static_assert(std::is_same_v<decltype(LayoutEntityFlipbook::m_flipbookPath),
                                 std::filesystem::path>);
    SUCCEED();
}
