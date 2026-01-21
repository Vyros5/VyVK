#pragma once

#include <filesystem>
#include <VyLib/STL/String.h>

#include <fstream>

namespace Vy
{
    using TPath = std::filesystem::path;

    namespace FS = std::filesystem;

    namespace Utils
    {
        static TString getPathWithoutFileName(const TPath& path) 
        {
            TPath withoutFilename{ TPath(path.parent_path()) };
            TString pathWithoutFilename = withoutFilename.string();
            if (!pathWithoutFilename.empty()) 
            {
                if (pathWithoutFilename.back() != '/') 
                {
                    pathWithoutFilename += '/';
                }
            }

            return pathWithoutFilename;
        }


        static bool fileExists(const TString& filename) 
        {
            std::ifstream infile(filename.c_str());
        
            return infile.good();
        }


        static bool isDirectory(const TString& filename) 
        {
            bool isDirectory = false;
            TPath path(filename);

            try {
                isDirectory = FS::is_directory(path);
            } 
            catch (...) {
                isDirectory = false;
            }

            return isDirectory;
        }
    }
}