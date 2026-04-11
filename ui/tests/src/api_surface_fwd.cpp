// Verifies that moth_ui_fwd.h is self-contained — it must compile without any
// other moth_ui header. Forward-declared types are used in function signatures
// to confirm every declaration in the file is parseable.

#include "moth_ui/moth_ui_fwd.h"

#include <catch2/catch_all.hpp>

// Use each forward-declared class in a function signature. These functions are
// never called; the compile step is what matters. Grouping them in an anonymous
// namespace suppresses unused-function warnings.
namespace {
    void pin_graphics(moth_ui::IRenderer* renderer, moth_ui::IImage* image, moth_ui::IFont* font,
                      moth_ui::ITarget* target, moth_ui::IFlipbook* flipbook,
                      moth_ui::IFlipbookFactory* flipbookFactory,
                      moth_ui::IImageFactory* imageFactory, moth_ui::IFontFactory* fontFactory,
                      moth_ui::FontFactory* fontFactoryImpl, moth_ui::NodeFactory* nodeFactory,
                      moth_ui::ILogger* logger, moth_ui::NullLogger* nullLogger) {
        (void)renderer; (void)image; (void)font; (void)target; (void)flipbook;
        (void)flipbookFactory; (void)imageFactory; (void)fontFactory;
        (void)fontFactoryImpl; (void)nodeFactory; (void)logger; (void)nullLogger;
    }

    void pin_events(moth_ui::Event* event, moth_ui::EventListener* listener,
                    moth_ui::EventDispatch* dispatch,
                    moth_ui::EventMouseDown* mouseDown, moth_ui::EventMouseUp* mouseUp,
                    moth_ui::EventMouseMove* mouseMove, moth_ui::EventMouseWheel* mouseWheel,
                    moth_ui::EventKey* key,
                    moth_ui::EventAnimation* animation,
                    moth_ui::EventAnimationStarted* animationStarted,
                    moth_ui::EventAnimationStopped* animationStopped,
                    moth_ui::EventFlipbookStarted* flipbookStarted,
                    moth_ui::EventFlipbookStopped* flipbookStopped) {
        (void)event; (void)listener; (void)dispatch;
        (void)mouseDown; (void)mouseUp; (void)mouseMove; (void)mouseWheel; (void)key;
        (void)animation; (void)animationStarted; (void)animationStopped;
        (void)flipbookStarted; (void)flipbookStopped;
    }

    void pin_nodes(moth_ui::Node* node, moth_ui::Group* group, moth_ui::NodeRect* nodeRect,
                   moth_ui::NodeImage* nodeImage, moth_ui::NodeText* nodeText,
                   moth_ui::NodeClip* nodeClip, moth_ui::NodeFlipbook* nodeFlipbook) {
        (void)node; (void)group; (void)nodeRect; (void)nodeImage;
        (void)nodeText; (void)nodeClip; (void)nodeFlipbook;
    }

    void pin_layout(moth_ui::LayoutEntity* layoutEntity,
                    moth_ui::LayoutEntityGroup* layoutGroup,
                    moth_ui::LayoutEntityRect* layoutRect,
                    moth_ui::LayoutEntityImage* layoutImage,
                    moth_ui::LayoutEntityText* layoutText,
                    moth_ui::LayoutEntityClip* layoutClip,
                    moth_ui::LayoutEntityRef* layoutRef,
                    moth_ui::LayoutEntityFlipbook* layoutFlipbook,
                    moth_ui::Layout* layout, moth_ui::LayoutCache* layoutCache,
                    moth_ui::LayoutRect* layoutRectObj) {
        (void)layoutEntity; (void)layoutGroup; (void)layoutRect; (void)layoutImage;
        (void)layoutText; (void)layoutClip; (void)layoutRef; (void)layoutFlipbook;
        (void)layout; (void)layoutCache; (void)layoutRectObj;
    }

    void pin_animation(moth_ui::AnimationTrack* animationTrack,
                       moth_ui::AnimationClip* animationClip,
                       moth_ui::AnimationEvent* animationEvent,
                       moth_ui::AnimationController* animationController,
                       moth_ui::AnimationTrackController* animationTrackController,
                       moth_ui::AnimationClipController* animationClipController,
                       moth_ui::DiscreteAnimationTrack* discreteTrack,
                       moth_ui::DiscreteAnimationTrackController* discreteTrackController,
                       moth_ui::ClipController* clipController) {
        (void)animationTrack; (void)animationClip; (void)animationEvent;
        (void)animationController; (void)animationTrackController;
        (void)animationClipController; (void)discreteTrack;
        (void)discreteTrackController; (void)clipController;
    }

    void pin_layers(moth_ui::Layer* layer, moth_ui::LayerStack* layerStack) {
        (void)layer; (void)layerStack;
    }

    void pin_context(moth_ui::Context* context) { (void)context; }

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
