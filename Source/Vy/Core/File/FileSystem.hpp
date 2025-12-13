#pragma once

#include <VyLib/VyLib.h>

#include <VyLib/STL/Path.h>

namespace Vy
{
    namespace FileSystem
    {
        /**
         * @brief Helper to read an asset file into a byte-array
         *
         * @param filePath The path to the file (relative to the assets directory)
         * @param count (optional) How many bytes to read. If 0 or not specified, the size
         * of the file will be used.
         * @return A vector filled with data read from the file
         */
        TBlob readAsset(const Path& filePath, U32 count = 0);


        /// @brief Reads a binary file and returns the contents as a vector of chars
        /// @param filePath Path to the file to read
        /// @return A vector of chars with the contents of the file or an empty vector if the file could not be read
        TVector<char> readBinary(const Path& filePath);

        /// @brief Reads a binary file and returns the contents as a vector of chars
        /// @param filePath Path to the file to read
        /// @param size Number of bytes to read
        /// @param offset Byte offset from the beginning of the file
        /// @return A vector with the contents of the file or an empty vector if the file could not be read
        TVector<char> readBinary(const Path& filePath, size_t size, size_t offset = 0);


        String getExtension(const String& uri);
    }
}