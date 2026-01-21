// #include <VyEngine/VK/Pipeline/Shader.h>

// #include <VyEngine/VK/Context.h>
// #include <VyLib/STL/Path.h>

// #include <iostream>
// #include <fstream>

// #include <shaderc/shaderc.hpp>

// namespace Vy
// {
// 	static std::string readFileToString(const TPath& filePath)
// 	{
// 		if (!FS::exists(filePath))
// 		{
// 			throw std::runtime_error("File does not exist: " + filePath.string());
// 		}

// 		std::ifstream file(filePath, std::ios::in | std::ios::binary);
// 		if (!file)
// 		{
// 			throw std::runtime_error("Failed to open file: " + filePath.string());
// 		}

// 		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
// 		return content;
// 	}


//     VyShader::VyShader(TString filePath, ShaderType type) : 
//         m_FilePath(std::move(filePath)), 
//         m_Type(type)
//     {
//         load();
        
//         TVector<U32> byteCode = compile();
//         m_Reflection = MakeShared<VyShaderReflection>(byteCode, getShaderStage());
        
//         createShaderModule( byteCode );
//     }

//     VyShader::~VyShader()
//     {
//         if (m_ShaderModule != VK_NULL_HANDLE)
//         {
//             vkDestroyShaderModule(VyContext::device(), m_ShaderModule, nullptr);
//         }
//     }

//     void VyShader::load()
//     {
//         try
//         {
//             m_ShaderSource = readFileToString( m_FilePath );
//         }
//         catch (const std::runtime_error& e)
//         {
//             throw std::runtime_error("Failed to load shader: " + TString(e.what()));
//         }
//     }


//     TVector<U32> VyShader::compile()
//     {
//         shaderc::Compiler       compiler;
//         shaderc::CompileOptions options;

//         options.SetOptimizationLevel(shaderc_optimization_level_zero);
//         options.SetGenerateDebugInfo();
//         options.SetSourceLanguage(shaderc_source_language_glsl);

//         shaderc_shader_kind kind;
//         switch (m_Type)
//         {
//             case ShaderType::Vertex:   kind = shaderc_vertex_shader; break;
//             case ShaderType::Fragment: kind = shaderc_fragment_shader; break;
//             case ShaderType::Compute:  kind = shaderc_compute_shader; break;
//             default: throw std::runtime_error("Unsupported shader type");
//         }

//         // Compile the shader
//         shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(m_ShaderSource, kind, m_FilePath.c_str(), options);

//         if (result.GetCompilationStatus() != shaderc_compilation_status_success)
//         {
//             throw std::runtime_error(result.GetErrorMessage());
//         }

//         return {result.cbegin(), result.cend()};
//     }


//     void VyShader::createShaderModule(const TVector<U32>& code)
//     {
//         VkShaderModuleCreateInfo createInfo{ VKInit::shaderModuleCreateInfo() };
//         {
//             createInfo.codeSize = code.size() * sizeof(U32);
//             createInfo.pCode    = code.data();
//         }

//         if (vkCreateShaderModule(VyContext::device(), &createInfo, nullptr, &m_ShaderModule) != VK_SUCCESS)
//         {
//             throw std::runtime_error("Failed to create shader module!");
//         }
//     }


//     VkShaderStageFlagBits VyShader::getShaderStage() const
//     {
//         switch (m_Type)
//         {
//             case ShaderType::Vertex:                 return VK_SHADER_STAGE_VERTEX_BIT;
//             case ShaderType::Fragment:               return VK_SHADER_STAGE_FRAGMENT_BIT;
//             case ShaderType::Compute:                return VK_SHADER_STAGE_COMPUTE_BIT;
//             case ShaderType::Geometry:               return VK_SHADER_STAGE_GEOMETRY_BIT;
//             case ShaderType::TessellationControl:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
//             case ShaderType::TessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
//             default:
//                 throw std::runtime_error("Unknown shader type");
//         }
//     }
// }