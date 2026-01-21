#pragma once

#include <VyLib/VyLib.h>

#include <VyLib/STL/Path.h>

namespace Vy
{
    namespace FileSystem
    {
        /// @brief Reads a binary file and returns the contents as a vector of chars
        /// @param filePath Path to the file to read
        /// @return A vector of chars with the contents of the file or an empty vector if the file could not be read
        TCharBlob readBinary(const TPath& filePath);

        /// @brief Reads a binary file and returns the contents as a vector of chars
        /// @param filePath Path to the file to read
        /// @param size Number of bytes to read
        /// @param offset Byte offset from the beginning of the file
        /// @return A vector with the contents of the file or an empty vector if the file could not be read
        TCharBlob readBinary(const TPath& filePath, size_t size, size_t offset = 0);


        bool readFileBinary(const TString& filepath, TCharBlob& result);

        bool writeFileBinary(const TString& filepath, const TCharBlob& data);

        bool writeFileBinary(const TString& filepath, const char* data, size_t dataSize);


        TString relativeEngineFilepath(const TString& baseFile, const TString& relativeFile);
    }
}