#pragma once

#include <chrono>

namespace Vy
{
    namespace Time
    {
        using Seconds = std::chrono::seconds;
        using SecondsPeriod = std::chrono::seconds::period;

        using Milliseconds       = std::chrono::milliseconds;
        using MillisecondsPeriod = std::chrono::milliseconds::period;

        template<class _Rep, class _Period>
        using Duration = std::chrono::duration<_Rep, _Period>;
    }
}