#pragma once

#include <Vy/Globals.h>
#include <chrono>
#include <thread>

namespace Vy
{
    class FrameRateController 
    {
    public:
        explicit FrameRateController(U32 targetFrameRate) :
            m_TargetFPS{ targetFrameRate }
        {
            m_TargetFrameDuration = std::chrono::duration<double>(1.0 / targetFrameRate);
        }

        // Returns the time in seconds since the last frame (dt)
        float waitForNextFrame()
        {
            // To avoid waiting:
            // auto  currentTime = std::chrono::high_resolution_clock::now(); // earlier
            // auto  newTime     = std::chrono::high_resolution_clock::now();
            // float dt          = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            // currentTime = newTime;

            auto now                = std::chrono::high_resolution_clock::now();
            auto timeSinceLastFrame = now - m_LastFrameTime;

            // Sleep until approximately just before the TargetFrameDuration
            if (timeSinceLastFrame < m_TargetFrameDuration - std::chrono::milliseconds(2)) 
            {
                std::this_thread::sleep_for(m_TargetFrameDuration - timeSinceLastFrame - std::chrono::milliseconds(2));
            }

            // Fine-tune with busy-waiting for the last small part
            while (timeSinceLastFrame < m_TargetFrameDuration) 
            {
                now = std::chrono::high_resolution_clock::now();
                
                timeSinceLastFrame = now - m_LastFrameTime;
            }

            m_LastFrameTime = std::chrono::high_resolution_clock::now();

            return std::chrono::duration<float>(timeSinceLastFrame).count();
        }

        // Set the target frame rate
        void setTargetFPS(U32 targetFPS)
        {
            m_TargetFPS           = targetFPS;
            m_TargetFrameDuration = std::chrono::duration<double>(1.0 / m_TargetFPS);
        }

    private:
        U32 m_TargetFPS;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_LastFrameTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double>                               m_TargetFrameDuration;
    };
}