#pragma once

#include "moth_graphics/utils/transform.h"
#include "moth_graphics/utils/vector.h"

#include <cmath>
#include <cstdint>

namespace moth::gfx {
    namespace {
        constexpr float kPi = 3.14159265f;
    }

    /**
     * @brief A 2D camera with position, zoom, rotation, follow, and shake.
     *
     * The camera's position is the world-space point at the centre of the view.
     * @c GetViewTransform() produces the world-to-screen matrix to apply via
     * @c IGraphics::SetTransform() before drawing the world. @c ScreenToWorld()
     * and @c WorldToScreen() convert between the two spaces, and @c Follow() /
     * @c Shake() drive the position smoothly over time.
     */
    class Camera {
    public:
        Camera() = default;

        /// @brief Returns the world-space position at the centre of the view.
        FloatVec2 const& GetPosition() const { return m_position; }

        /// @brief Sets the world-space position at the centre of the view.
        void SetPosition(FloatVec2 position) { m_position = position; }

        /// @brief Moves the camera by @p delta (world units).
        void Move(FloatVec2 delta) { m_position += delta; }

        /// @brief Snaps the camera to @p target (world units).
        void SnapTo(FloatVec2 target) { m_position = target; }

        /// @brief Smoothly moves the camera toward @p target using exponential damping.
        ///
        /// @param target    Desired world-space centre.
        /// @param dt        Elapsed time in seconds.
        /// @param smoothing Follow speed. Higher values track faster; <= 0 snaps immediately.
        void Follow(FloatVec2 target, float dt, float smoothing) {
            if (smoothing <= 0.0f || dt <= 0.0f) {
                m_position = target;
                return;
            }
            float const factor = 1.0f - std::exp(-smoothing * dt);
            m_position += (target - m_position) * factor;
        }

        /// @brief Returns the zoom factor (1 = 1:1 pixels).
        float GetZoom() const { return m_zoom; }

        /// @brief Sets the zoom factor. Values must be positive.
        void SetZoom(float zoom) { m_zoom = zoom > 0.0f ? zoom : 0.0f; }

        /// @brief Returns the rotation in degrees (clockwise).
        float GetRotation() const { return m_rotation; }

        /// @brief Sets the rotation in degrees (clockwise).
        void SetRotation(float rotation) { m_rotation = rotation; }

        /// @brief Advances shake state and produces the frame's shake offset.
        ///
        /// Call once per frame with the elapsed time in seconds.
        void Update(float dt);

        /// @brief Starts a camera shake.
        ///
        /// The offset decays linearly to zero over @p duration seconds.
        /// @param intensity Peak displacement in world units.
        /// @param duration  Shake lifetime in seconds.
        void Shake(float intensity, float duration);

        /// @brief Returns the current shake offset (world units), applied automatically by @c GetViewTransform().
        FloatVec2 const& GetShakeOffset() const { return m_shakeOffset; }

        /// @brief Returns the world-to-screen transform for a @p viewportSize view.
        ///
        /// @param viewportSize Size of the viewport in logical pixels.
        FloatMat4x4 GetViewTransform(FloatVec2 viewportSize) const;

        /// @brief Converts a screen-space point (logical pixels) to world space.
        FloatVec2 ScreenToWorld(FloatVec2 screenPos, FloatVec2 viewportSize) const;

        /// @brief Converts a world-space point to screen space (logical pixels).
        FloatVec2 WorldToScreen(FloatVec2 worldPos, FloatVec2 viewportSize) const;

        /// @brief Returns the world-space rectangle visible for a @p viewportSize view.
        ///
        /// Accounts for rotation; returns the axis-aligned bounds of the rotated view.
        void GetViewportBounds(FloatVec2 viewportSize, FloatVec2& topLeft, FloatVec2& bottomRight) const;

    private:
        FloatVec2 EffectivePosition() const { return m_position + m_shakeOffset; }

        float NextRandom01();
        FloatMat4x4 BuildView(FloatVec2 viewportSize) const;

        FloatVec2 m_position = { 0.0f, 0.0f };
        float m_zoom = 1.0f;
        float m_rotation = 0.0f;

        FloatVec2 m_shakeOffset = { 0.0f, 0.0f };
        float m_shakeIntensity = 0.0f;
        float m_shakeDuration = 0.0f;
        float m_shakeTime = 0.0f;
        uint32_t m_rngState = 0x12345678u;
    };

    inline void Camera::Update(float dt) {
        if (m_shakeTime > 0.0f && m_shakeDuration > 0.0f) {
            m_shakeTime -= dt;
            if (m_shakeTime <= 0.0f) {
                m_shakeTime = 0.0f;
                m_shakeOffset = { 0.0f, 0.0f };
            } else {
                float const falloff = m_shakeTime / m_shakeDuration;
                float const magnitude = m_shakeIntensity * falloff;
                float const angle = NextRandom01() * (2.0f * kPi);
                float const distance = NextRandom01() * magnitude;
                m_shakeOffset = { std::cos(angle) * distance, std::sin(angle) * distance };
            }
        }
    }

    inline void Camera::Shake(float intensity, float duration) {
        if (intensity <= 0.0f || duration <= 0.0f) {
            return;
        }
        m_shakeIntensity = intensity;
        m_shakeDuration = duration;
        m_shakeTime = duration;
    }

    inline float Camera::NextRandom01() {
        // Simple LCG: good enough for shake jitter, avoids global RNG state.
        m_rngState = m_rngState * 1664525u + 1013904223u;
        return static_cast<float>(m_rngState >> 8) / static_cast<float>(1u << 24);
    }

    inline FloatMat4x4 Camera::BuildView(FloatVec2 viewportSize) const {
        FloatVec2 const center = viewportSize * 0.5f;
        return FloatMat4x4::Translation(center)
             * FloatMat4x4::Rotation(m_rotation, { 0.0f, 0.0f })
             * FloatMat4x4::Scale(FloatVec2{ m_zoom, m_zoom })
             * FloatMat4x4::Translation(FloatVec2{ -EffectivePosition().x, -EffectivePosition().y });
    }

    inline FloatMat4x4 Camera::GetViewTransform(FloatVec2 viewportSize) const {
        return BuildView(viewportSize);
    }

    inline FloatVec2 Camera::WorldToScreen(FloatVec2 worldPos, FloatVec2 viewportSize) const {
        return BuildView(viewportSize).TransformPoint(worldPos);
    }

    inline FloatVec2 Camera::ScreenToWorld(FloatVec2 screenPos, FloatVec2 viewportSize) const {
        FloatVec2 const center = viewportSize * 0.5f;
        float const invZoom = m_zoom != 0.0f ? 1.0f / m_zoom : 0.0f;
        return (FloatMat4x4::Translation(EffectivePosition())
                * FloatMat4x4::Scale(FloatVec2{ invZoom, invZoom })
                * FloatMat4x4::Rotation(-m_rotation, { 0.0f, 0.0f })
                * FloatMat4x4::Translation(FloatVec2{ -center.x, -center.y }))
            .TransformPoint(screenPos);
    }

    inline void Camera::GetViewportBounds(FloatVec2 viewportSize, FloatVec2& topLeft, FloatVec2& bottomRight) const {
        // The four view corners in world space bound the visible region.
        FloatVec2 const corners[4] = {
            ScreenToWorld({ 0.0f, 0.0f }, viewportSize),
            ScreenToWorld({ viewportSize.x, 0.0f }, viewportSize),
            ScreenToWorld({ 0.0f, viewportSize.y }, viewportSize),
            ScreenToWorld({ viewportSize.x, viewportSize.y }, viewportSize),
        };
        topLeft = corners[0];
        bottomRight = corners[0];
        for (auto const& corner : corners) {
            topLeft.x = std::min(topLeft.x, corner.x);
            topLeft.y = std::min(topLeft.y, corner.y);
            bottomRight.x = std::max(bottomRight.x, corner.x);
            bottomRight.y = std::max(bottomRight.y, corner.y);
        }
    }
}
