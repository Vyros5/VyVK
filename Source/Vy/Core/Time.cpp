#include <Vy/Core/Time.h>

#include <algorithm>

namespace Vy
{
    // --- Static member initialization ---
    Time::TimePoint Time::m_StartTime;
    Time::TimePoint Time::m_LastFrameTime;
    Time::TimePoint Time::m_CurrentFrameTime;
    F32             Time::m_DeltaTime          = 0.0f;
    F32             Time::m_TotalTime          = 0.0f;
    F32             Time::m_UnscaledDeltaTime  = 0.0f;
    F32             Time::m_UnscaledTotalTime  = 0.0f;
    U64             Time::m_FrameCount         = 0;
    F32             Time::m_TimeScale          = 1.0f;
    F32             Time::m_FPS                = 0.0f;
    F32             Time::m_AverageFrameTimeMs = 0.0f;
    F32             Time::m_FPSAccumulator     = 0.0f;
    U32             Time::m_FPSFrameCount      = 0;
    Time::TimePoint Time::m_FPSLastUpdateTime;

    // --- Initialization ---

    void Time::init() 
    {
        m_StartTime         = Clock::now();
        m_LastFrameTime     = m_StartTime;
        m_CurrentFrameTime  = m_StartTime;
        m_FPSLastUpdateTime = m_StartTime;

        m_DeltaTime          = 0.0f;
        m_TotalTime          = 0.0f;
        m_UnscaledDeltaTime  = 0.0f;
        m_UnscaledTotalTime  = 0.0f;
        m_FrameCount         = 0;
        m_TimeScale          = 1.0f;
        m_FPS                = 0.0f;
        m_AverageFrameTimeMs = 0.0f;
        m_FPSAccumulator     = 0.0f;
        m_FPSFrameCount      = 0;
    }

    // --- Update ---

    void Time::update() 
    {
        // Update time points
        m_LastFrameTime = m_CurrentFrameTime;
        m_CurrentFrameTime = Clock::now();

        // Compute unscaled delta time
        Duration frameDuration = m_CurrentFrameTime - m_LastFrameTime;
        m_UnscaledDeltaTime = frameDuration.count();

        // Clamp delta time to prevent spiral of death (max 0.1 seconds = 10 FPS minimum)
        m_UnscaledDeltaTime = std::min(m_UnscaledDeltaTime, 0.1f);

        // Apply time scale
        m_DeltaTime = m_UnscaledDeltaTime * m_TimeScale;

        // Update total time
        m_UnscaledTotalTime += m_UnscaledDeltaTime;
        m_TotalTime += m_DeltaTime;

        // Increment frame count
        ++m_FrameCount;

        // Update FPS counter (every second)
        m_FPSAccumulator += m_UnscaledDeltaTime;
        ++m_FPSFrameCount;

        Duration fpsDuration = m_CurrentFrameTime - m_FPSLastUpdateTime;
        F32 fpsDurationSeconds = fpsDuration.count();

        if (fpsDurationSeconds >= 1.0f) 
        {
            // Compute FPS and average frame time
            m_FPS = static_cast<F32>(m_FPSFrameCount) / fpsDurationSeconds;
            m_AverageFrameTimeMs = (m_FPSAccumulator / static_cast<F32>(m_FPSFrameCount)) * 1000.0f;

            // Reset accumulators
            m_FPSAccumulator    = 0.0f;
            m_FPSFrameCount     = 0;
            m_FPSLastUpdateTime = m_CurrentFrameTime;
        }
    }
}