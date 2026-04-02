# Flipbook Node Implementation Plan

A `NodeFlipbook` node that renders a single frame from a sprite sheet. The
frame advances on an internal timer driven by the node's own `Update()` call,
independently of the keyframe animation system. Position, color, rotation, and
other layout properties are still keyframe-animated through the existing system.

Flipbook metadata lives in a separate descriptor file that the application
loads via a factory, mirroring how texture atlas packs work. moth_ui defines
only the data struct and the factory interface; the file format and parsing live
entirely in moth_graphics.

---

## Descriptor file format

A `.flipbook.json` file sits alongside (or separately from) the sprite sheet.
It describes the sheet layout and playback parameters. Example:

```json
{
  "image": "assets/hero.png",
  "frame_width": 64,
  "frame_height": 64,
  "frame_cols": 8,
  "frame_rows": 4,
  "max_frames": 30,
  "fps": 12,
  "loop": true
}
```

- `image` is a path relative to the descriptor file.
- `max_frames` allows the usable frame count to be less than `cols * rows` when
  the sheet is not fully packed.
- `loop` controls whether playback restarts after the last frame.

The layout entity in a moth_ui layout file stores only the path to this
descriptor:

```json
{
  "type": "Flipbook",
  "flipbook_path": "assets/hero.flipbook.json"
}
```

---

## Part 1: moth_ui

### Step 1 — Add `Flipbook` to `LayoutEntityType`

**File:** `include/moth_ui/layout/layout_entity_type.h`

Add a new enumerator after `Clip`:

```cpp
Flipbook, ///< LayoutEntityFlipbook.
```

---

### Step 2 — Define `IFlipbook`

**New file:** `include/moth_ui/graphics/iflipbook.h`

An abstract interface that exposes flipbook metadata and the underlying image.
Follows the same pattern as `IImage` and `IFont`. The concrete implementation
lives in moth_graphics; moth_ui never sees it.

```cpp
namespace moth_ui {
    class IFlipbook {
    public:
        // Sheet-wide metadata: frame pixel size, grid layout, total usable frames, clip count.
        struct SheetDesc {
            IntVec2 FrameDimensions; // width and height of a single frame in pixels
            IntVec2 SheetCells;      // grid size: x = cols, y = rows
            int MaxFrames = 0;
            int NumClips  = 0;
        };

        // Per-clip loop behaviour when the last frame is reached.
        enum class LoopType { Stop, Reset, Loop };

        // Per-clip settings: inclusive frame range, playback rate, loop behaviour.
        struct ClipDesc {
            int Start = 0;
            int End   = 0;
            int FPS   = 0;
            LoopType Loop = LoopType::Stop;
        };

        virtual IImage& GetImage() const = 0;
        virtual void GetSheetDesc(SheetDesc& outDesc) const = 0;
        virtual std::string_view GetClipName(int index) const = 0;
        virtual bool GetClipDesc(std::string_view name, ClipDesc& outDesc) const = 0;

        IFlipbook() = default;
        IFlipbook(IFlipbook const&) = default;
        IFlipbook(IFlipbook&&) = default;
        IFlipbook& operator=(IFlipbook const&) = default;
        IFlipbook& operator=(IFlipbook&&) = default;
        virtual ~IFlipbook() = default;
    };
}
```

---

### Step 3 — Define `IFlipbookFactory`

**New file:** `include/moth_ui/iflipbook_factory.h`

Follows the same pattern as `IImageFactory`:

```cpp
namespace moth_ui {
    class IFlipbookFactory {
    public:
        IFlipbookFactory() = default;
        IFlipbookFactory(IFlipbookFactory const&) = default;
        IFlipbookFactory(IFlipbookFactory&&) = default;
        IFlipbookFactory& operator=(IFlipbookFactory const&) = default;
        IFlipbookFactory& operator=(IFlipbookFactory&&) = default;
        virtual ~IFlipbookFactory() = default;

        virtual void FlushCache() = 0;

        /// @brief Loads a flipbook from a descriptor file.
        /// @param path Path to the .flipbook.json descriptor.
        /// @return Loaded flipbook, or nullptr on failure.
        virtual std::unique_ptr<IFlipbook> GetFlipbook(std::filesystem::path const& path) = 0;
    };
}
```

---

### Step 4 — Add `IFlipbookFactory` to `Context`

**File:** `include/moth_ui/context.h` and `src/context.cpp`

Add `IFlipbookFactory* flipbookFactory = nullptr` as an optional parameter.
Because not all applications use flipbooks, accept it as a defaulted pointer
rather than making it required:

```cpp
Context(IImageFactory* imageFactory,
        IFontFactory* fontFactory,
        IRenderer* renderer,
        IFlipbookFactory* flipbookFactory = nullptr);
```

Add a getter:

```cpp
IFlipbookFactory* GetFlipbookFactory() const { return m_flipbookFactory; }
```

Note: unlike the image and font factories, this getter returns a raw pointer
that may be null. Callers (i.e. `NodeFlipbook`) must check before using it.

---

### Step 5 — Create `LayoutEntityFlipbook`

**New files:**
- `include/moth_ui/layout/layout_entity_flipbook.h`
- `src/layout/layout_entity_flipbook.cpp`

Inherits from `LayoutEntity`. Stores only the path to the descriptor file.

**Header:**

```cpp
class LayoutEntityFlipbook : public LayoutEntity {
public:
    explicit LayoutEntityFlipbook(LayoutRect const& initialBounds);
    explicit LayoutEntityFlipbook(LayoutEntityGroup* parent);

    std::shared_ptr<LayoutEntity> Clone(CloneType cloneType) override;
    LayoutEntityType GetType() const override { return LayoutEntityType::Flipbook; }
    std::unique_ptr<Node> Instantiate(Context& context) override;
    nlohmann::json Serialize(SerializeContext const& context) const override;
    bool Deserialize(nlohmann::json const& json, SerializeContext const& context) override;

    std::filesystem::path m_flipbookPath;
};
```

**Implementation:**

- `Serialize`: call `LayoutEntity::Serialize(context)` to get the base JSON
  object, then append `"flipbook_path"`.
- `Deserialize`: call `LayoutEntity::Deserialize(json, context)` first, then
  read `m_flipbookPath` with `json.value("flipbook_path", "")`.
- `Clone`: copy-construct into a `shared_ptr<LayoutEntityFlipbook>`.
- `Instantiate`: `return std::make_unique<NodeFlipbook>(context, std::static_pointer_cast<LayoutEntityFlipbook>(shared_from_this()))`.

---

### Step 6 — Register the new entity type in `CreateLayoutEntity`

**File:** `src/layout/layout_entity.cpp`

Add the include:

```cpp
#include "moth_ui/layout/layout_entity_flipbook.h"
```

Add a case to the `CreateLayoutEntity` switch:

```cpp
case LayoutEntityType::Flipbook:
    return std::make_unique<LayoutEntityFlipbook>(nullptr);
```

---

### Step 7 — Add flipbook event types

**File:** `include/moth_ui/events/event.h`

Add two new type codes to the `EventType` enum after `EVENTTYPE_ANIMATION_STOPPED`:

```cpp
EVENTTYPE_FLIPBOOK_STARTED = 8, ///< EventFlipbookStarted.
EVENTTYPE_FLIPBOOK_STOPPED = 9, ///< EventFlipbookStopped.
```

**New file:** `include/moth_ui/events/event_flipbook.h`

Two inline event classes following the same pattern as `EventAnimationStarted`
and `EventAnimationStopped` in `event_animation.h`:

```cpp
class EventFlipbookStarted : public Event {
public:
    EventFlipbookStarted(NodeFlipbook* node, std::string_view clipName)
        : Event(GetStaticType()), m_node(node), m_clipName(clipName) {}
    static constexpr int GetStaticType() { return EVENTTYPE_FLIPBOOK_STARTED; }
    NodeFlipbook* GetNode() const { return m_node; }
    std::string_view GetClipName() const { return m_clipName; }
    std::unique_ptr<Event> Clone() const override {
        return std::make_unique<EventFlipbookStarted>(m_node, m_clipName);
    }
private:
    NodeFlipbook* m_node = nullptr;
    std::string m_clipName;
};

class EventFlipbookStopped : public Event {
public:
    EventFlipbookStopped(NodeFlipbook* node, std::string_view clipName)
        : Event(GetStaticType()), m_node(node), m_clipName(clipName) {}
    static constexpr int GetStaticType() { return EVENTTYPE_FLIPBOOK_STOPPED; }
    NodeFlipbook* GetNode() const { return m_node; }
    std::string_view GetClipName() const { return m_clipName; }
    std::unique_ptr<Event> Clone() const override {
        return std::make_unique<EventFlipbookStopped>(m_node, m_clipName);
    }
private:
    NodeFlipbook* m_node = nullptr;
    std::string m_clipName;
};
```

No separate `.cpp` is needed; all methods are inline.

---

### Step 8 — Create `NodeFlipbook`

**New files:**
- `include/moth_ui/nodes/node_flipbook.h`
- `src/nodes/node_flipbook.cpp`

Inherits from `Node`. Follows the structure of `NodeImage`.

**Members:**

```cpp
std::unique_ptr<IFlipbook> m_flipbook; // null until loaded
int m_currentFrame    = 0;
float m_accumulatedMs = 0.0f;
bool m_playing        = false;
```

**`ReloadEntityInternal()` override:**

Cast the entity to `LayoutEntityFlipbook`. Ask the context for the flipbook
factory. If the factory is null or the path is empty, clear `m_flipbook` and
return. Otherwise call `factory->GetFlipbook(entity->m_flipbookPath)` and store
the result. Reset `m_currentFrame` and `m_accumulatedMs` to zero. If loading
succeeded and `m_flipbook` is not null, set `m_playing = true` and send
`EventFlipbookStarted(this)`.

**`Update(uint32_t ticks)` override:**

Call `Node::Update(ticks)` first, then advance the timer:

```cpp
if (!m_playing || !m_flipbook || m_flipbook->GetFps() <= 0) return;

float const frameDurationMs = 1000.0f / static_cast<float>(m_flipbook->GetFps());
m_accumulatedMs += static_cast<float>(ticks);

while (m_accumulatedMs >= frameDurationMs) {
    m_accumulatedMs -= frameDurationMs;
    ++m_currentFrame;
    if (m_currentFrame >= m_flipbook->GetMaxFrames()) {
        if (m_flipbook->GetLoop()) {
            m_currentFrame = 0;
        } else {
            m_currentFrame = m_flipbook->GetMaxFrames() - 1;
            m_playing = false;
            SendEvent(EventFlipbookStopped(this));
            break;
        }
    }
}
```

**`DrawInternal()` override:**

If `m_flipbook` is null, return early. Compute the source rect for the current
frame and call `RenderImage`. The renderer already accepts a `sourceRect` in
`IRenderer::RenderImage`, so no renderer changes are needed:

```cpp
int const col = m_currentFrame % m_flipbook->GetFrameCols();
int const row = m_currentFrame / m_flipbook->GetFrameCols();
IntRect const srcRect = MakeRect(col * m_flipbook->GetFrameWidth(),
                                 row * m_flipbook->GetFrameHeight(),
                                 m_flipbook->GetFrameWidth(),
                                 m_flipbook->GetFrameHeight());
```

Look at `NodeImage::DrawInternal()` in `src/nodes/node_image.cpp` for how to
obtain the destination rect and how to push/pop color and blend mode around the
draw call.

---

### Step 9 — Register `NodeFlipbook` in `NodeFactory`

**File:** `src/node_factory.cpp`

Add includes:

```cpp
#include "moth_ui/nodes/node_flipbook.h"
#include "moth_ui/layout/layout_entity_flipbook.h"
```

In `NodeFactory::Create(Context&, shared_ptr<LayoutEntity>)`, add a case for
`LayoutEntityType::Flipbook` following the same pattern as the `Image` case:

```cpp
case LayoutEntityType::Flipbook:
    return std::make_unique<NodeFlipbook>(
        context,
        std::static_pointer_cast<LayoutEntityFlipbook>(entity));
```

---

### Step 10 — Expose new types in public headers

**File:** `include/moth_ui/moth_ui.h`

Add the new includes in their respective sections:

```cpp
// events
#include "moth_ui/events/event_flipbook.h"

// graphics
#include "moth_ui/graphics/iflipbook.h"

// layout
#include "moth_ui/layout/layout_entity_flipbook.h"

// nodes
#include "moth_ui/nodes/node_flipbook.h"

// root
#include "moth_ui/iflipbook_factory.h"
```

**File:** `include/moth_ui/moth_ui_fwd.h`

Add forward declarations:

```cpp
class IFlipbook;
class IFlipbookFactory;
class LayoutEntityFlipbook;
class NodeFlipbook;
class EventFlipbookStarted;
class EventFlipbookStopped;
```

---

### Step 11 — Update CMakeLists.txt (moth_ui)

Add the new source and header files to the target:

```text
include/moth_ui/graphics/iflipbook.h
include/moth_ui/iflipbook_factory.h
include/moth_ui/events/event_flipbook.h
include/moth_ui/layout/layout_entity_flipbook.h
src/layout/layout_entity_flipbook.cpp
include/moth_ui/nodes/node_flipbook.h
src/nodes/node_flipbook.cpp
```

---

### Step 12 — Remove `LoadTexturePack` from `IImageFactory` (related cleanup)

Now that `IFlipbookFactory` establishes the correct pattern — application-level
loading concerns belong in concrete classes, not in moth_ui's abstract
interfaces — remove `LoadTexturePack` from `IImageFactory`:

- **`include/moth_ui/iimage_factory.h`**: remove the `LoadTexturePack` pure
  virtual declaration.
- **`tests/src/mock_context.h`**: remove the stub override.
- **moth_graphics `MothImageFactory`**: remove the `override` keyword from the
  declaration; the method still exists on the concrete `ImageFactory` class and
  can be called directly there.

---

## Part 2: moth_graphics

### Step 13 — Define the concrete `Flipbook` class and implement `FlipbookFactory`

**New files:**
- `include/moth_graphics/graphics/flipbook.h`
- `include/moth_graphics/graphics/flipbook_factory.h`
- `src/graphics/flipbook_factory.cpp`

**`Flipbook`** is the concrete implementation of `moth_ui::IFlipbook`. It owns
the loaded image and stores the parsed metadata:

```cpp
class Flipbook : public moth_ui::IFlipbook {
public:
    Flipbook(std::unique_ptr<IImage> image,
             int frameWidth, int frameHeight,
             int frameCols, int frameRows,
             int maxFrames, int fps, bool loop);

    moth_ui::IImage& GetImage() const override { return *m_image; }
    int GetFrameWidth()  const override { return m_frameWidth; }
    int GetFrameHeight() const override { return m_frameHeight; }
    int GetFrameCols()   const override { return m_frameCols; }
    int GetFrameRows()   const override { return m_frameRows; }
    int GetMaxFrames()   const override { return m_maxFrames; }
    int GetFps()         const override { return m_fps; }
    bool GetLoop()       const override { return m_loop; }

private:
    std::unique_ptr<IImage> m_image;
    int m_frameWidth, m_frameHeight;
    int m_frameCols, m_frameRows;
    int m_maxFrames, m_fps;
    bool m_loop;
};
```

**`FlipbookFactory`** parses descriptor files and constructs `Flipbook` instances:

```cpp
class FlipbookFactory {
public:
    explicit FlipbookFactory(AssetContext& assetContext);

    void FlushCache();

    /// @brief Parses a .flipbook.json descriptor and loads the sprite sheet.
    /// @param path Path to the descriptor file.
    /// @return Loaded Flipbook, or nullptr on failure. Failures are logged
    ///         via spdlog.
    std::unique_ptr<moth_ui::IFlipbook> GetFlipbook(std::filesystem::path const& path);

private:
    AssetContext& m_assetContext;
};
```

The `GetFlipbook` implementation:
1. Check the file exists; log and return nullptr if not.
2. Parse the JSON; log and return nullptr on parse failure.
3. Resolve the `"image"` path relative to the descriptor file's parent directory.
4. Load the image via `m_assetContext.ImageFromFile(imagePath)`; log and return
   nullptr if loading fails.
5. Construct and return a `Flipbook` with all parsed fields.

Caching by path (similar to how `ImageFactory` caches) can be added later;
start without it.

---

### Step 14 — Implement `MothFlipbookFactory` bridge adapter

**New files:**
- `include/moth_graphics/graphics/moth_ui/moth_flipbook_factory.h`
- `src/graphics/moth_ui/moth_flipbook_factory.cpp`

Thin adapter that implements `moth_ui::IFlipbookFactory` by delegating to
`FlipbookFactory`. Follows the same pattern as `MothImageFactory`:

```cpp
class MothFlipbookFactory : public moth_ui::IFlipbookFactory {
public:
    explicit MothFlipbookFactory(graphics::FlipbookFactory& factoryImpl);

    void FlushCache() override;
    std::unique_ptr<moth_ui::IFlipbook> GetFlipbook(
        std::filesystem::path const& path) override;

private:
    graphics::FlipbookFactory& m_factoryImpl;
};
```

---

### Step 15 — Wire up the factory in `AssetContext` and `Context`

**File:** `include/moth_graphics/graphics/asset_context.h`

Add a `GetFlipbookFactory()` pure virtual method alongside the existing
`GetImageFactory()` and `GetFontFactory()`:

```cpp
virtual FlipbookFactory& GetFlipbookFactory() = 0;
```

Update the concrete `AssetContext` implementation(s) to own a `FlipbookFactory`
member and return it from `GetFlipbookFactory()`.

Wherever a `moth_ui::Context` is constructed (search for `moth_ui::Context{` or
`new moth_ui::Context`), add the `MothFlipbookFactory` as the fourth argument.

---

### Step 16 — Update CMakeLists.txt (moth_graphics)

Add the new source and header files to the target:

```text
include/moth_graphics/graphics/flipbook.h
include/moth_graphics/graphics/flipbook_factory.h
src/graphics/flipbook_factory.cpp
include/moth_graphics/graphics/moth_ui/moth_flipbook_factory.h
src/graphics/moth_ui/moth_flipbook_factory.cpp
```

---

## Part 3: moth_editor

### Step 17 — Add "Add Flipbook" to the canvas element creation menu

**File:** `src/editor/panels/editor_panel_canvas.cpp`

Find where the other "Add X" menu items are created (e.g. "Add Image", "Add
Rect"). Add:

```cpp
if (ImGui::MenuItem("Add Flipbook")) {
    // create a default LayoutEntityFlipbook, add it to the root group,
    // wrap in an editor action, call PerformEditAction
    // follow the exact same pattern as the existing "Add Image" item
}
```

Include `moth_ui/layout/layout_entity_flipbook.h` at the top of the file.

---

### Step 18 — Add `DrawFlipbookProperties` to the properties panel

**File:** `src/editor/panels/editor_panel_properties.cpp`

Add includes:

```cpp
#include "moth_ui/layout/layout_entity_flipbook.h"
#include "moth_ui/nodes/node_flipbook.h"
```

Add a `DrawFlipbookProperties` method following the same pattern as
`DrawImageProperties`. It receives a `shared_ptr<NodeFlipbook>` and draws:

1. **Descriptor path** — a read-only text field showing the current path with a
   "Browse..." button that opens an NFD file dialog. On selection, update
   `entity->m_flipbookPath` and call `node->ReloadEntity()`, wrapped in a
   `MakeChangeValueAction` for undo support.

In `DrawNodeProperties`, add a case to the switch:

```cpp
case moth_ui::LayoutEntityType::Flipbook:
    DrawFlipbookProperties(std::static_pointer_cast<moth_ui::NodeFlipbook>(node));
    break;
```

Note: the frame dimensions, FPS, and other parameters are authored in the
descriptor file, not in the editor properties panel. The properties panel only
needs to let the user point the node at a descriptor.

---

### Step 19 — Ensure the preview panel advances flipbook timers

**File:** `src/editor/panels/editor_panel_preview.cpp`

During preview playback the preview panel calls `Update()` on the root group.
Verify this call propagates down the node tree via virtual dispatch. If
`Group::Update` already iterates children and calls `child->Update(ticks)`, no
change is needed. If it does not, add the propagation.

---

## Testing checklist

- Create a `.flipbook.json` descriptor file and a matching sprite sheet. Load
  the descriptor from application code via `IFlipbookFactory::GetFlipbook` and
  verify all fields are populated correctly.
- Create a new layout in the editor, add a Flipbook node, and assign the
  descriptor path. Save and reload the layout; verify the path round-trips
  correctly through JSON.
- Open the preview panel and confirm the flipbook animates at the rate specified
  in the descriptor.
- Load the layout in a test application and verify `EventFlipbookStopped` fires
  when `loop: false` and the last frame is reached.
- Confirm that position, color, and rotation keyframe animation on the flipbook
  node work alongside its own internal playback timer.
- Confirm undo/redo works for the descriptor path change in the properties panel.
- Verify that `GetFlipbook` returns nullptr and logs a clear error when the
  descriptor file does not exist or the image path inside it is invalid.
