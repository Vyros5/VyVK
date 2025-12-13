#pragma once

#include <VyLib/Core/Assert.h>
#include <VyLib/Core/Defines.h>
#include <VyLib/Common/Numeric.h>

#include <exception>

#include <fmt/format.h>

namespace Vy
{
    // Generic exception
    class VyException : public std::exception 
    {
    public:

        template<typename... Args>
        VyException(fmt::format_string<Args...> msg, Args&&... args) : 
            m_Message() 
        { 
            try 
            {
                m_Message = fmt::format(msg, std::forward<Args>(args)...);
            } 
            catch (...) 
            { 
                // If an exception is throw in the format function store a replacment string.
                m_Message = "VyException in exception formater!";
            }
        }


        VyException() : 
            m_Message("VyEngine VyException") 
        {
        }
        
        virtual const char* what() const throw() 
        {
            return m_Message.c_str();   
        }
        
    private:
        // Store the exception message
        String m_Message;
    };
}