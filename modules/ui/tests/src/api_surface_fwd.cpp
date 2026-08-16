// Verifies that moth_ui_fwd.h is self-contained — it must compile without any
// other moth_ui header. Forward-declared types are used in function signatures
// to confirm every declaration in the file is parseable.

#include "moth_ui/moth_ui_fwd.h"

#include <catch2/catch_all.hpp>

// Use each forward-declared class in a function signature. These functions are
// never called; the compile step is what matters. Grouping them in an anonymous
// namespace suppresses unused-function warnings.
namespace {
    void pin_graphics(moth::ui::IRenderer* renderer, moth::ui::IImage* image, moth::ui::IFont* font,
                      moth::ui::ITarget* target, moth::ui::IFlipbook* flipbook,
                      moth::ui::IFlipbookFactory* flipbookFactory,
                      moth::ui::IImageFactory* imageFactory, moth::ui::IFontFactory* fontFactory,
                      moth::ui::FontFactory* fontFactoryImpl, moth::ui::NodeFactory* nodeFactory,
                      moth::ui::ILogger* logger, moth::ui::NullLogger* nullLogger) {
        (void)renderer; (void)image; (void)font; (void)target; (void)flipbook;
        (void)flipbookFactory; (void)imageFactory; (void)fontFactory;
        (void)fontFactoryImpl; (void)nodeFactory; (void)logger; (void)nullLogger;
    }

    void pin_events(moth::ui::Event* event, moth::ui::IEventListener* listener,
                    moth::ui::EventDispatch* dispatch,
                    moth::ui::EventMouseDown* mouseDown, moth::ui::EventMouseUp* mouseUp,
                    moth::ui::EventMouseMove* mouseMove, moth::ui::EventMouseWheel* mouseWheel,
                    moth::ui::EventKey* key,
                    moth::ui::EventAnimation* animation,
                    moth::ui::EventAnimationStarted* animationStarted,
                    moth::ui::EventAnimationStopped* animationStopped,
                    moth::ui::EventFlipbookStarted* flipbookStarted,
                    moth::ui::EventFlipbookStopped* flipbookStopped) {
        (void)event; (void)listener; (void)dispatch;
        (void)mouseDown; (void)mouseUp; (void)mouseMove; (void)mouseWheel; (void)key;
        (void)animation; (void)animationStarted; (void)animationStopped;
        (void)flipbookStarted; (void)flipbookStopped;
    }

    void pin_nodes(moth::ui::Node* node, moth::ui::Group* group, moth::ui::NodeRect* nodeRect,
                   moth::ui::NodeImage* nodeImage, moth::ui::NodeText* nodeText,
                   moth::ui::NodeClip* nodeClip, moth::ui::NodeFlipbook* nodeFlipbook) {
        (void)node; (void)group; (void)nodeRect; (void)nodeImage;
        (void)nodeText; (void)nodeClip; (void)nodeFlipbook;
    }

    void pin_layout(moth::ui::LayoutEntity* layoutEntity,
                    moth::ui::LayoutEntityGroup* layoutGroup,
                    moth::ui::LayoutEntityRect* layoutRect,
                    moth::ui::LayoutEntityImage* layoutImage,
                    moth::ui::LayoutEntityText* layoutText,
                    moth::ui::LayoutEntityClip* layoutClip,
                    moth::ui::LayoutEntityRef* layoutRef,
                    moth::ui::LayoutEntityFlipbook* layoutFlipbook,
                    moth::ui::Layout* layout, moth::ui::LayoutCache* layoutCache,
                    moth::ui::LayoutRect* layoutRectObj) {
        (void)layoutEntity; (void)layoutGroup; (void)layoutRect; (void)layoutImage;
        (void)layoutText; (void)layoutClip; (void)layoutRef; (void)layoutFlipbook;
        (void)layout; (void)layoutCache; (void)layoutRectObj;
    }

    void pin_animation(moth::ui::AnimationTrack* animationTrack,
                       moth::ui::AnimationClip* animationClip,
                       moth::ui::AnimationMarker* animationEvent,
                       moth::ui::AnimationController* animationController,
                       moth::ui::AnimationTrackController* animationTrackController,
                       moth::ui::AnimationClipController* animationClipController,
                       moth::ui::DiscreteAnimationTrack* discreteTrack,
                       moth::ui::DiscreteAnimationTrackController* discreteTrackController) {
        (void)animationTrack; (void)animationClip; (void)animationEvent;
        (void)animationController; (void)animationTrackController;
        (void)animationClipController; (void)discreteTrack;
        (void)discreteTrackController;
    }

    void pin_layers(moth::ui::Layer* layer, moth::ui::LayerStack* layerStack) {
        (void)layer; (void)layerStack;
    }

    void pin_context(moth::ui::Context* context) { (void)context; }

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
