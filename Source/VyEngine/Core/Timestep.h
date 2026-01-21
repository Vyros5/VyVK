#pragma once

#include <VyLib/VyLib.h>
#include <VyLib/STL/Chrono.h>

namespace Vy
{
    class VyTimestep 
    {
    public:

        using SecondsDuration      = Time::Duration<float, Time::SecondsPeriod>;
        using MillisecondsDuration = Time::Duration<float, Time::MillisecondsPeriod>;

        VyTimestep(SecondsDuration time);

        VyTimestep& operator=(const SecondsDuration& timestep);
        VyTimestep& operator-=(const VyTimestep& other);
        VyTimestep operator-(const VyTimestep& other) const;
        bool operator<=(const SecondsDuration& other) const;
        operator float() const { return m_Timestep.count(); }

        void print() const;
        static float count() { return m_Timestep.count(); }
        SecondsDuration      getSeconds() const;
        MillisecondsDuration getMilliseconds() const;

    private:
        static SecondsDuration m_Timestep;
    };
}