#pragma once

#include <VyLib/VyLib.h>

#include <chrono>

namespace Vy
{
    /**
     * Time - Global time management system
     *
     * Design principles:
     * - Static singleton pattern for global access
     * - High-resolution timing using std::chrono
     * - Call update() once per frame to compute deltaTime
     * - Thread-safe reads (no mutex needed since writes only happen in update())
     * - Provides both F32 (seconds) and U64 (frame count) time values
     *
     * Usage:
     *   Time::init();
     *
     *   // In main loop:
     *   Time::update();
     *
     *   // In systems:
     *   F32 dt = Time::deltaTime();
     *   F32 t = Time::totalTime();
     *   U64 frame = Time::frameCount();
     */
    class Time 
    {
    public:
        using Clock     = std::chrono::high_resolution_clock;
        using TimePoint = Clock::time_point;
        using Duration  = std::chrono::duration<F32>;

        // --- Initialization ---

        /**
         * Initialize time system
         * Must be called before first update()
         */
        static void init();

        /**
         * Update time values (call once per frame at start of frame)
         * Computes deltaTime and updates totalTime and frameCount
         */
        static void update();

        // --- Time Queries ---

        /**
         * Get delta time since last frame in seconds
         *
         * @return Time elapsed since last update() call
         */
        static F32 deltaTime() { return m_DeltaTime; }

        /**
         * Get total time since init() in seconds
         *
         * @return Total elapsed time
         */
        static F32 totalTime() { return m_TotalTime; }

        /**
         * Get current frame count
         *
         * @return Number of frames since init()
         */
        static U64 frameCount() { return m_FrameCount; }

        /**
         * Get unscaled delta time (ignores time scale)
         *
         * @return Real delta time in seconds
         */
        static F32 unscaledDeltaTime() { return m_UnscaledDeltaTime; }

        /**
         * Get unscaled total time (ignores time scale)
         *
         * @return Real total time in seconds
         */
        static F32 unscaledTotalTime() { return m_UnscaledTotalTime; }

        // --- Time Scale ---

        /**
         * Set time scale factor
         * Affects deltaTime() and totalTime() but not unscaled versions
         *
         * @param scale Time scale (1.0 = normal, 0.5 = half speed, 2.0 = double speed)
         */
        static void setTimeScale(F32 scale) { m_TimeScale = scale; }

        /**
         * Get current time scale
         *
         * @return Time scale factor
         */
        static F32 timeScale() { return m_TimeScale; }

        // --- FPS ---

        /**
         * Get current frames per second (smoothed over last second)
         *
         * @return Approximate FPS
         */
        static F32 fps() { return m_FPS; }

        /**
         * Get average frame time in milliseconds (smoothed over last second)
         *
         * @return Average frame time in ms
         */
        static F32 averageFrameTime() { return m_AverageFrameTimeMs; }

    private:
        // Time points
        static TimePoint m_StartTime;
        static TimePoint m_LastFrameTime;
        static TimePoint m_CurrentFrameTime;

        // Time values (scaled)
        static F32 m_DeltaTime;
        static F32 m_TotalTime;

        // Time values (unscaled)
        static F32 m_UnscaledDeltaTime;
        static F32 m_UnscaledTotalTime;

        // Frame count
        static U64 m_FrameCount;

        // Time scale
        static F32 m_TimeScale;

        // FPS tracking
        static F32 m_FPS;
        static F32 m_AverageFrameTimeMs;
        static F32 m_FPSAccumulator;
        static U32   m_FPSFrameCount;
        static TimePoint m_FPSLastUpdateTime;

        // Prevent instantiation
        Time() = delete;
    };
}