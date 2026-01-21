#include <VyEngine/Core/Timestep.h>

using namespace std::literals::chrono_literals;

namespace Vy
{
    VyTimestep::SecondsDuration VyTimestep::m_Timestep = 0ms;

    VyTimestep::VyTimestep(SecondsDuration time) 
    {
    }

    VyTimestep& VyTimestep::operator=(const SecondsDuration& timestep) 
    {
        this->m_Timestep = timestep;
        return *this;
    }

    VyTimestep& VyTimestep::operator-=(const VyTimestep& other) 
    {
        m_Timestep = m_Timestep - other.m_Timestep;
        return *this;
    }

    VyTimestep VyTimestep::operator-(const VyTimestep& other) const 
    {
        return m_Timestep - other.m_Timestep;
    }

    bool VyTimestep::operator<=(const SecondsDuration& other) const 
    {
        return (m_Timestep - other) <= 0ms;
    }

    VyTimestep::SecondsDuration VyTimestep::getSeconds() const 
    {
        return m_Timestep;
    }

    VyTimestep::MillisecondsDuration VyTimestep::getMilliseconds() const 
    {
        return (MillisecondsDuration)m_Timestep;
    }

    void VyTimestep::print() const 
    {
        auto inMilliSeconds = getMilliseconds();
        VY_INFO_TAG("VyTimestep", "Timestep in milli-seconds: {0} ms", inMilliSeconds.count());

        auto inSeconds = getSeconds();
        VY_INFO_TAG("VyTimestep", "Timestep in seconds: {0} s", inSeconds.count());
    }
}