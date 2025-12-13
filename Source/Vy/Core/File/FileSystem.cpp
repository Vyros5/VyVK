#include <Vy/Core/File/FileSystem.hpp>

#include <fstream>

namespace Vy
{
	TBlob read_binary_file(const Path& filePath, U32 count) 
	{
		TBlob data;

		std::ifstream file;

		file.open(filePath, std::ios::in | std::ios::binary);

		if (!file.is_open()) 
		{
			throw std::runtime_error("Failed to open file: " + filePath.string());
		}

		U64 readCount = count;
		if (count == 0) 
		{
			file.seekg(0, std::ios::end);
			readCount = static_cast<U64>(file.tellg());
			file.seekg(0, std::ios::beg);
		}

		data.resize(static_cast<size_t>(readCount));
		file.read(reinterpret_cast<char*>(data.data()), readCount);
		file.close();

		return data;
	}


	TBlob FileSystem::readAsset(const Path& filePath, U32 count)
	{
		return read_binary_file(filePath, count);
	}

	String FileSystem::getExtension(const String& uri) 
	{
		auto dotPos = uri.find_last_of('.');
		
		if (dotPos == String::npos) 
		{
			throw std::runtime_error{"Uri has no extension"};
		}

		return uri.substr(dotPos + 1);
	}


	TVector<char> FileSystem::readBinary(const Path& filePath)
	{
		std::ifstream file{ filePath, std::ios::ate | std::ios::binary };
		if (!file.is_open())
			return {};

		auto fileSize = static_cast<size_t>(file.tellg());
		TVector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
    

	TVector<char> FileSystem::readBinary(const Path& filePath, size_t size, size_t offset)
	{
		std::ifstream file{ filePath, std::ios::binary };
		if (!file.is_open())
			return {};

		file.seekg(offset);
		TVector<char> buffer(size);
		file.read(buffer.data(), size);
		file.close();

		return buffer;
	}
}