#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    struct DataBuffer 
    {
        U8*   Bytes = nullptr;
        USize Size = 0;

        DataBuffer() = default;

        DataBuffer(USize size) : 
            Size(size) 
        {
            Bytes = (U8*) malloc(Size);
        }

        // Constructor that copies data
        DataBuffer(const void* data, const USize size) : 
            Size(size) 
        {
            Bytes = (U8*) malloc(Size);
        
            if (Bytes) 
            {
                memcpy(Bytes, data, Size);
            } 
            else 
            {
                this->Size = 0; // Indicate allocation failure
            }
        }

        ~DataBuffer() 
        {
            release();
        }

        // Copy Constructor (Deep Copy)
        DataBuffer(const DataBuffer& other) : 
            Size(other.Size) 
        {
            if (other.Bytes) 
            {
                Bytes = (U8*) malloc(Size);
            
                if (Bytes) 
                {
                    memcpy(Bytes, other.Bytes, Size);
                } 
                else
                {
                    Size = 0; 
                }
            } 
            else 
            {
                Bytes = nullptr;
                Size  = 0;
            }
        }

        // Copy Assignment Operator (Rule of Three/Five)
        DataBuffer& operator=(const DataBuffer& other) 
        {
            if (this != &other) 
            { 
                release();

                Size = other.Size;

                if (other.Bytes) 
                {
                    Bytes = (U8*) malloc(Size);

                    if (Bytes) 
                    {
                        memcpy(Bytes, other.Bytes, Size);
                    }
                    else 
                    {
                        Size = 0; 
                    }
                } 
                else 
                {
                    Bytes = nullptr;
                    Size  = 0;
                }
            }

            return *this;
        }


        operator bool() const 
        {
            return Bytes != nullptr && Size > 0;
        }

        void release() 
        {
            if (Bytes) 
            {
                free(Bytes);

                Bytes = nullptr;
                Size  = 0;
            }
        }

        template<typename T>
        T* get() const 
        {
            return (T*)Bytes;
        }

        USize size() const 
        {
            return Size;
        }
    };
}