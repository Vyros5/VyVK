#include <VyEngine/Core/File/FileSystem.hpp>

#include <fstream>

namespace Vy
{
	TCharBlob FileSystem::readBinary(const TPath& filePath)
	{
		std::ifstream file{ filePath, std::ios::ate | std::ios::binary };

		if (!file.is_open())
			return {};

		auto fileSize = static_cast<size_t>(file.tellg());
		TCharBlob buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}
    

	TCharBlob FileSystem::readBinary(const TPath& filePath, size_t size, size_t offset)
	{
		std::ifstream file{ filePath, std::ios::binary };
		
		if (!file.is_open())
			return {};

		file.seekg(offset);
		TCharBlob buffer(size);
		file.read(buffer.data(), size);
		file.close();

		return buffer;
	}


	bool FileSystem::readFileBinary(const TString& filepath, TCharBlob& result)
	{
        std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

        if (!file.is_open()) return false;

        size_t fileSize = static_cast<size_t>(file.tellg());
        result.resize(fileSize);

        file.seekg(0);
        file.read(result.data(), fileSize);

        file.close();
        return true;
	}

	bool FileSystem::writeFileBinary(const TString& filepath, const TCharBlob& data)
	{
        std::ofstream file{ filepath, std::ios::out | std::ios::binary };

        if (!file.is_open()) return false;

        file.write(data.data(), data.size());
        file.flush();

        file.close();
        return true;
	}

	bool FileSystem::writeFileBinary(const TString& filepath, const char* data, size_t dataSize)
	{
        std::ofstream file{ filepath, std::ios::out | std::ios::binary };

        if (!file.is_open()) return false;

        file.write(data, dataSize);
        file.flush();

        file.close();
        return true;
	}


	TString FileSystem::relativeEngineFilepath(const TString& baseFile, const TString& relativeFile)
	{
        FS::path basePath{ baseFile };
        FS::path relPath{ relativeFile };
        FS::path combinedPath = FS::canonical(basePath.parent_path() / relPath);
        FS::path cleanedPath = combinedPath.make_preferred();
        FS::path enginePath = FS::relative(cleanedPath);

        return enginePath.string();
	}
}