// Verifies that moth_ui_fwd.h is self-contained — it must compile without any
// other moth_ui header. Forward-declared types are used in function signatures
// to confirm every declaration in the file is parseable.

#include "moth_ui/moth_ui_fwd.h"

#include <catch2/catch_all.hpp>

// Use each forward-declared class in a function signature. These functions are
// never called; the compile step is what matters. Grouping them in an anonymous
// namespace suppresses unused-function warnings.
namespace {
    void pin_graphics(moth_ui::IRenderer*, moth_ui::IImage*, moth_ui::IFont*,
                      moth_ui::ITarget*, moth_ui::IFlipbook*, moth_ui::IFlipbookFactory*,
                      moth_ui::IImageFactory*, moth_ui::IFontFactory*, moth_ui::FontFactory*,
                      moth_ui::NodeFactory*, moth_ui::ILogger*, moth_ui::NullLogger*) {}

    void pin_events(moth_ui::Event*, moth_ui::EventListener*, moth_ui::EventDispatch*,
                    moth_ui::EventMouseDown*, moth_ui::EventMouseUp*,
                    moth_ui::EventMouseMove*, moth_ui::EventMouseWheel*,
                    moth_ui::EventKey*,
                    moth_ui::EventAnimation*, moth_ui::EventAnimationStarted*,
                    moth_ui::EventAnimationStopped*,
                    moth_ui::EventFlipbookStarted*, moth_ui::EventFlipbookStopped*) {}

    void pin_nodes(moth_ui::Node*, moth_ui::Group*, moth_ui::NodeRect*,
                   moth_ui::NodeImage*, moth_ui::NodeText*, moth_ui::NodeClip*,
                   moth_ui::NodeFlipbook*) {}

    void pin_layout(moth_ui::LayoutEntity*, moth_ui::LayoutEntityGroup*,
                    moth_ui::LayoutEntityRect*, moth_ui::LayoutEntityImage*,
                    moth_ui::LayoutEntityText*, moth_ui::LayoutEntityClip*,
                    moth_ui::LayoutEntityRef*, moth_ui::LayoutEntityFlipbook*,
                    moth_ui::Layout*, moth_ui::LayoutCache*, moth_ui::LayoutRect*) {}

    void pin_animation(moth_ui::AnimationTrack*, moth_ui::AnimationClip*,
                       moth_ui::AnimationEvent*, moth_ui::AnimationController*,
                       moth_ui::AnimationTrackController*,
                       moth_ui::AnimationClipController*,
                       moth_ui::DiscreteAnimationTrack*,
                       moth_ui::DiscreteAnimationTrackController*,
                       moth_ui::ClipController*) {}

    void pin_layers(moth_ui::Layer*, moth_ui::LayerStack*) {}

    void pin_context(moth_ui::Context*) {}

    // Suppress unused-function warnings without suppressing the pin checks above.
    void use_all() {
        (void)&pin_graphics;
        (void)&pin_events;
        (void)&pin_nodes;
        (void)&pin_layout;
        (void)&pin_animation;
        (void)&pin_layers;
        (void)&pin_context;
    }
}

TEST_CASE("moth_ui_fwd.h compiles standalone and declares all public types", "[api][fwd]") {
    (void)&use_all;
    SUCCEED();
}
