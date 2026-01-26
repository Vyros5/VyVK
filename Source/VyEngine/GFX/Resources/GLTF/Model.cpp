#include <VyEngine/GFX/Resources/GLTF/Model.h>

#include "tiny_gltf.h"


// #define TINYGLTF_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
// #define TINYGLTF_NO_STB_IMAGE_WRITE
// #define STBI_MSC_SECURE_CRT

// #include <tiny_obj_loader.h>
#include <VyLib/Util/Hash.h>
#include <VyLib/Core/Math.h>
#include <VyEngine/Globals.h>
#include <iostream>
#include <VyLib/Common/AnsiColor.h>
#include <VyLib/Util/String.h>

namespace std 
{
    template <>
    struct hash<Vy::VyVertex> 
    {
        size_t operator()(Vy::VyVertex const& vertex) const 
        {
            size_t seed = 0;
            
            Vy::Hash::hashCombine(seed
                , vertex.Position
                , vertex.Color
                , vertex.Normal
                , vertex.UV
                , vertex.Tangent
                // , vertex.Bitangent
            );
            
            return seed;
        }
    };
}

namespace Vy
{

    // static inline float DecodeComponent(int componentType, bool normalized, const void* src)
    // {
    //     switch (componentType) 
    //     {
    //     case TINYGLTF_COMPONENT_TYPE_FLOAT: 
    //     {
    //         float v;
    //         std::memcpy(&v, src, sizeof(float));
    //         return v;
    //     }
    //     case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: 
    //     {
    //         U8 v;
    //         std::memcpy(&v, src, 1);
    //         return normalized ? (float)v / 255.0f : (float)v;
    //     }
    //     case TINYGLTF_COMPONENT_TYPE_BYTE: 
    //     {
    //         I8 v;
    //         std::memcpy(&v, src, 1);
    //         if (normalized) {
    //             // Map to [-1,1]
    //             return std::max(-1.0f, (float)v / 127.0f);
    //         }
    //         return (float)v;
    //     }
    //     case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: 
    //     {
    //         U16 v;
    //         std::memcpy(&v, src, 2);
    //         return normalized ? (float)v / 65535.0f : (float)v;
    //     }
    //     case TINYGLTF_COMPONENT_TYPE_SHORT: 
    //     {
    //         I16 v;
    //         std::memcpy(&v, src, 2);
    //         if (normalized) {
    //             return std::max(-1.0f, (float)v / 32767.0f);
    //         }
    //         return (float)v;
    //     }
    //     case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: 
    //     {
    //         U32 v;
    //         std::memcpy(&v, src, 4);
    //         return (float)v; // POSITION should not be UINT normally, but handle generically.
    //     }
    //     default:
    //         VY_ERROR("Unsupported vertex componentType: {}", componentType);
    //         return 0.0f;
    //     }
    // }

    // // https://github.com/ClemensX/ShadedPathV/blob/a2525bc341c7e4ecd49f820871f5c04f6171fa10/src/lib/gltf.cpp
    // void extractVertexAttribute(
    //     const tinygltf::Model&     model, 
    //     const tinygltf::Primitive& primitive, 
    //     const TString&             attributeName, 
    //     TVector<float>&            outData, 
    //     int&                       stride) 
    // {
    //     outData.clear();
    //     stride = 0;

    //     auto it = primitive.attributes.find(attributeName);
    //     if (it == primitive.attributes.end()) return;

    //     const tinygltf::Accessor& accessor = model.accessors[static_cast<size_t>(it->second)];
    //     if (accessor.bufferView < 0) 
    //     {
    //         VY_ERROR("Accessor bufferView < 0 for attribute {}", attributeName);
    //         return;
    //     }

    //     const tinygltf::BufferView& bufferView = model.bufferViews[static_cast<size_t>(accessor.bufferView)];
    //     const tinygltf::Buffer&     buffer     = model.buffers[static_cast<size_t>(bufferView.buffer)];

    //     const size_t numComponents = tinygltf::GetNumComponentsInType(accessor.type);
    //     if (numComponents == 0) 
    //     {
    //         VY_ERROR("Invalid accessor.type for attribute {}", attributeName);
    //         return;
    //     }

    //     // Bytes per single component.
    //     int componentSize = 0;
    //     switch (accessor.componentType) 
    //     {
    //     case TINYGLTF_COMPONENT_TYPE_FLOAT:          componentSize = 4; break;
    //     case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    //     case TINYGLTF_COMPONENT_TYPE_BYTE:           componentSize = 1; break;
    //     case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    //     case TINYGLTF_COMPONENT_TYPE_SHORT:          componentSize = 2; break;
    //     case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:   componentSize = 4; break;
    //     default:
    //         VY_ERROR("Unsupported vertex componentType: {}", accessor.componentType);
    //         return;
    //     }

    //     // Byte stride between consecutive vertices in the buffer.
    //     const int byteStride = accessor.ByteStride(bufferView) > 0
    //         ? accessor.ByteStride(bufferView)
    //         : int(numComponents) * componentSize;

    //     if (byteStride < componentSize * int(numComponents)) 
    //     {
    //         // Spec allows >= packed size. Anything smaller is invalid.
    //         VY_ERROR("Invalid byteStride ({}) for attribute {}", byteStride, attributeName);
    //         return;
    //     }

    //     const size_t start = bufferView.byteOffset + accessor.byteOffset;
    //     // Last vertex must fit: start + (count-1)*byteStride + packedSize
    //     const size_t packedSize = numComponents * size_t(componentSize);
    //     const size_t lastByte   = start + (accessor.count ? (accessor.count - 1) * size_t(byteStride) : 0) + packedSize;
    //     if (lastByte > buffer.data.size()) 
    //     {
    //         VY_ERROR("Buffer overrun risk while reading attribute  {}", attributeName);
    //         return;
    //     }

    //     const unsigned char* pBase = reinterpret_cast<const unsigned char*>(buffer.data.data() + start);

    //     outData.resize(accessor.count * numComponents);

    //     stride = int(numComponents); // number of float components per vertex

    //     for (size_t i = 0; i < accessor.count; ++i) 
    //     {
    //         const unsigned char* pElem = pBase + i * byteStride;

    //         // Components in the attribute are ALWAYS tightly packed starting at pElem,
    //         // even if vertex is interleaved (extra bytes follow after the attribute data).
    //         for (size_t c = 0; c < numComponents; ++c) 
    //         {
    //             const void* pCompSrc = pElem + c * componentSize;

    //             outData[i * numComponents + c] = DecodeComponent(accessor.componentType, accessor.normalized, pCompSrc);
    //         }
    //     }
    // }


    // VkFormat VyGLTFModel::accessorFormat(tinygltf::Model& inModel, U32 index) const
    // {
    //     assert(index < inModel.accessors.size());
    //     auto& accessor = inModel.accessors[index];

    //     VkFormat format;
    //     switch (accessor.componentType) 
    //     {
    //         case TINYGLTF_COMPONENT_TYPE_BYTE:
    //         {
    //             static const TMap<int, VkFormat> mapped_format = {
    //                 {TINYGLTF_TYPE_SCALAR,       VK_FORMAT_R8_SINT},
    //                 {  TINYGLTF_TYPE_VEC2,     VK_FORMAT_R8G8_SINT},
    //                 {  TINYGLTF_TYPE_VEC3,   VK_FORMAT_R8G8B8_SINT},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SINT}
    //             };

    //             static const TMap<int, VkFormat> mapped_format_normalize = {
    //                 {TINYGLTF_TYPE_SCALAR,       VK_FORMAT_R8_SNORM},
    //                 {  TINYGLTF_TYPE_VEC2,     VK_FORMAT_R8G8_SNORM},
    //                 {  TINYGLTF_TYPE_VEC3,   VK_FORMAT_R8G8B8_SNORM},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_SNORM}
    //             };

    //             if (accessor.normalized) 
    //             {
    //                 format = mapped_format_normalize.at(accessor.type);
    //             } 
    //             else {
    //                 format = mapped_format.at(accessor.type);
    //             }
                
    //         } break;

    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    //         {
    //             static const TMap<int, VkFormat> mapped_format = {
    //                 {TINYGLTF_TYPE_SCALAR,       VK_FORMAT_R8_UINT},
    //                 {  TINYGLTF_TYPE_VEC2,     VK_FORMAT_R8G8_UINT},
    //                 {  TINYGLTF_TYPE_VEC3,   VK_FORMAT_R8G8B8_UINT},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UINT}
    //             };

    //             static const TMap<int, VkFormat> mapped_format_normalize = {
    //                 {TINYGLTF_TYPE_SCALAR,       VK_FORMAT_R8_UNORM},
    //                 {  TINYGLTF_TYPE_VEC2,     VK_FORMAT_R8G8_UNORM},
    //                 {  TINYGLTF_TYPE_VEC3,   VK_FORMAT_R8G8B8_UNORM},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R8G8B8A8_UNORM}
    //             };
                
    //             if (accessor.normalized) 
    //             {
    //                 format = mapped_format_normalize.at(accessor.type);
    //             } 
    //             else {
    //                 format = mapped_format.at(accessor.type);
    //             }

    //         } break;

    //         case TINYGLTF_COMPONENT_TYPE_SHORT:
    //         {
    //             static const TMap<int, VkFormat> mapped_format = {
    //                 {TINYGLTF_TYPE_SCALAR,          VK_FORMAT_R16_SINT},
    //                 {  TINYGLTF_TYPE_VEC2,       VK_FORMAT_R16G16_SINT},
    //                 {  TINYGLTF_TYPE_VEC3,    VK_FORMAT_R16G16B16_SINT},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_SINT}
    //             };

    //             static const TMap<int, VkFormat> mapped_format_normalize = {
    //                 {TINYGLTF_TYPE_SCALAR,          VK_FORMAT_R16_SNORM},
    //                 {  TINYGLTF_TYPE_VEC2,       VK_FORMAT_R16G16_SNORM},
    //                 {  TINYGLTF_TYPE_VEC3,    VK_FORMAT_R16G16B16_SNORM},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_SNORM}
    //             };

    //             if (accessor.normalized) 
    //             {
    //                 format = mapped_format_normalize.at(accessor.type);
    //             } 
    //             else {
    //                 format = mapped_format.at(accessor.type);
    //             }

    //         } break;

    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
    //         {
    //             static const TMap<int, VkFormat> mapped_format = {
    //                 {TINYGLTF_TYPE_SCALAR,          VK_FORMAT_R16_UINT},
    //                 {  TINYGLTF_TYPE_VEC2,       VK_FORMAT_R16G16_UINT},
    //                 {  TINYGLTF_TYPE_VEC3,    VK_FORMAT_R16G16B16_UINT},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UINT}
    //             };

    //             static const TMap<int, VkFormat> mapped_format_normalize = {
    //                 {TINYGLTF_TYPE_SCALAR,          VK_FORMAT_R16_UNORM},
    //                 {  TINYGLTF_TYPE_VEC2,       VK_FORMAT_R16G16_UNORM},
    //                 {  TINYGLTF_TYPE_VEC3,    VK_FORMAT_R16G16B16_UNORM},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R16G16B16A16_UNORM}
    //             };

    //             if (accessor.normalized) 
    //             {
    //                 format = mapped_format_normalize.at(accessor.type);
    //             } 
    //             else {
    //                 format = mapped_format.at(accessor.type);
    //             }

    //         } break;

    //         case TINYGLTF_COMPONENT_TYPE_INT:
    //         {
    //             static const TMap<int, VkFormat> mapped_format = {
    //                 {TINYGLTF_TYPE_SCALAR,          VK_FORMAT_R32_SINT},
    //                 {  TINYGLTF_TYPE_VEC2,       VK_FORMAT_R32G32_SINT},
    //                 {  TINYGLTF_TYPE_VEC3,    VK_FORMAT_R32G32B32_SINT},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SINT}
    //             };

    //             format = mapped_format.at(accessor.type);

    //         } break;

    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
    //         {
    //             static const TMap<int, VkFormat> mapped_format = {
    //                 {TINYGLTF_TYPE_SCALAR,          VK_FORMAT_R32_UINT},
    //                 {  TINYGLTF_TYPE_VEC2,       VK_FORMAT_R32G32_UINT},
    //                 {  TINYGLTF_TYPE_VEC3,    VK_FORMAT_R32G32B32_UINT},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_UINT}
    //             };

    //             format = mapped_format.at(accessor.type);

    //         } break;

    //         case TINYGLTF_COMPONENT_TYPE_FLOAT:
    //         {
    //             static const TMap<int, VkFormat> mapped_format = {
    //                 {TINYGLTF_TYPE_SCALAR,          VK_FORMAT_R32_SFLOAT},
    //                 {  TINYGLTF_TYPE_VEC2,       VK_FORMAT_R32G32_SFLOAT},
    //                 {  TINYGLTF_TYPE_VEC3,    VK_FORMAT_R32G32B32_SFLOAT},
    //                 {  TINYGLTF_TYPE_VEC4, VK_FORMAT_R32G32B32A32_SFLOAT}
    //             };

    //             format = mapped_format.at(accessor.type);

    //         } break;

    //         default:
    //         {
    //             format = VK_FORMAT_UNDEFINED;
    //             VY_ERROR("Invalid gltf accessor component type: {}, {}", accessor.componentType, accessor.type);
            
    //         } break;
    //     }
    //     return format;
    // }


    // VkFormat VyGLTFModel::imageFormat(tinygltf::Model& inModel, U32 index) const
    // {
    //     assert(index < inModel.images.size());
        
    //     auto& image = inModel.images[index];

    //     int component = image.component;
    //     int pixel_type = image.pixel_type;

    //     if (1 == component) {
    //         switch (pixel_type) {
    //         case TINYGLTF_COMPONENT_TYPE_BYTE: return VK_FORMAT_R8_SNORM;
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return VK_FORMAT_R8_UNORM;
    //         case TINYGLTF_COMPONENT_TYPE_SHORT: return VK_FORMAT_R16_SNORM;
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return VK_FORMAT_R16_UNORM;
    //         case TINYGLTF_COMPONENT_TYPE_INT:
    //         case TINYGLTF_COMPONENT_TYPE_FLOAT:
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return VK_FORMAT_R32_SFLOAT;
    //         };
    //     } else if (2 == component) {
    //         switch (pixel_type) {
    //         case TINYGLTF_COMPONENT_TYPE_BYTE: return VK_FORMAT_R8G8_SNORM;
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return VK_FORMAT_R8G8_UNORM;
    //         case TINYGLTF_COMPONENT_TYPE_SHORT: return VK_FORMAT_R16G16_SNORM;
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return VK_FORMAT_R16G16_UNORM;
    //         case TINYGLTF_COMPONENT_TYPE_INT:
    //         case TINYGLTF_COMPONENT_TYPE_FLOAT:
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return VK_FORMAT_R32G32_SFLOAT;
    //         };
    //     } else if (3 == component) {
    //         switch (pixel_type) {
    //         case TINYGLTF_COMPONENT_TYPE_BYTE: return VK_FORMAT_R8G8B8_SNORM;
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return VK_FORMAT_R8G8B8_UNORM;
    //         case TINYGLTF_COMPONENT_TYPE_SHORT: return VK_FORMAT_R16G16B16_SNORM;
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return VK_FORMAT_R16G16B16_UNORM;
    //         case TINYGLTF_COMPONENT_TYPE_INT:
    //         case TINYGLTF_COMPONENT_TYPE_FLOAT:
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return VK_FORMAT_R32G32B32_SFLOAT;
    //         };
    //     } else if (4 == component) {
    //         switch (pixel_type) {
    //         case TINYGLTF_COMPONENT_TYPE_BYTE: return VK_FORMAT_R8G8B8A8_SNORM;
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return VK_FORMAT_R8G8B8A8_UNORM;
    //         case TINYGLTF_COMPONENT_TYPE_SHORT: return VK_FORMAT_R16G16B16A16_SNORM;
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return VK_FORMAT_R16G16B16A16_UNORM;
    //         case TINYGLTF_COMPONENT_TYPE_INT:
    //         case TINYGLTF_COMPONENT_TYPE_FLOAT:
    //         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: return VK_FORMAT_R32G32B32A32_SFLOAT;
    //         };
    //     }
    //     VY_ERROR("Invalid gltf image: component = {}, pixel type = {}", component, pixel_type);
    //     return VK_FORMAT_UNDEFINED;
    // }

    // // Helper: Get accessor data as typed pointer
    // template<typename T>
    // const T* getAccessorData(const tinygltf::Model& model, int accessorIndex) 
    // {
    //     if (accessorIndex < 0) return nullptr;

    //     const auto& accessor   = model.accessors  [ accessorIndex       ];
    //     const auto& bufferView = model.bufferViews[ accessor.bufferView ];
    //     const auto& buffer     = model.buffers    [ bufferView.buffer   ];

    //     return reinterpret_cast<const T*>( &buffer.data[ bufferView.byteOffset + accessor.byteOffset ] );
    // }

    
    // // Helper: Get accessor count
    // size_t getAccessorCount(const tinygltf::Model& model, int accessorIndex) 
    // {
    //     if (accessorIndex < 0) return 0;
    
    //     return model.accessors[ accessorIndex ].count;
    // }













    // Shared<VyGLTFModel> VyGLTFModel::createFromFile(const TString& filepath)
    // {
    //     Builder builder{};
    //     builder.loadModel( filepath );
        
    //     return MakeShared<VyGLTFModel>( builder );
    // }


    // void VyGLTFModel::Builder::loadModel(const TString& filepath) 
    // {
    //     try {
    //         std::cout << "Loading model: " << filepath << std::endl;
            
    //         TString extension = filepath.substr(filepath.find_last_of(".") + 1);
            
    //         if (extension == "gltf" || extension == "glb") 
    //         {
    //             std::cout << "Detected GLTF format" << std::endl;
    //             loadGltfModel(filepath);
    //         } 
    //         else {
    //             std::cout << "Detected OBJ format" << std::endl;
    //             loadObjModel(filepath);
    //         }
    //     } 
    //     catch (const std::exception& e) 
    //     {
    //         std::cerr << "Error loading model: " << e.what() << std::endl;
    //         throw;
    //     }
    // }


    // void VyGLTFModel::Builder::loadObjModel(const TString &filePath) 
    // {
    //     tinyobj::attrib_t attrib;
    //     std::vector<tinyobj::shape_t> shapes;
    //     std::vector<tinyobj::material_t> materials;
    //     TString warn, err;

    //     if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath.c_str())) 
    //     {
    //         throw std::runtime_error(warn + err);
    //     }

    //     Vertices.clear();
    //     Indices .clear();

    //     std::unordered_map<VyVertex, uint32_t> uniqueVertices{};
    //     for (const auto &shape : shapes) 
    //     {
    //         for (const auto &index : shape.mesh.indices) 
    //         {
    //             VyVertex vertex{};

    //             if (index.vertex_index >= 0) 
    //             {
    //                 vertex.Position = {
    //                     attrib.vertices[3 * index.vertex_index + 0],
    //                     attrib.vertices[3 * index.vertex_index + 1],
    //                     attrib.vertices[3 * index.vertex_index + 2]
    //                 };

    //                 vertex.Color = {
    //                     attrib.colors[3 * index.vertex_index + 0],
    //                     attrib.colors[3 * index.vertex_index + 1],
    //                     attrib.colors[3 * index.vertex_index + 2],
    //                 };
    //             }

    //             if (index.normal_index >= 0) 
    //             {
    //                 vertex.Normal = {
    //                     attrib.normals[3 * index.normal_index + 0],
    //                     attrib.normals[3 * index.normal_index + 1],
    //                     attrib.normals[3 * index.normal_index + 2]
    //                 };
    //             }

    //             if (index.texcoord_index >= 0) 
    //             {
    //                 vertex.UV = {
    //                     attrib.texcoords[2 * index.texcoord_index + 0],
    //                     attrib.texcoords[2 * index.texcoord_index + 1]
    //                 };
    //             }
    //             if(uniqueVertices.count( vertex ) == 0)
    //             {
    //                 uniqueVertices[ vertex ] = static_cast<U32>(Vertices.size());
                    
    //                 Vertices.push_back( vertex );
    //             }
                
    //             Indices.push_back( uniqueVertices[ vertex ] );
    //         }
    //     }
    // }


    // void VyGLTFModel::Builder::loadGltfModel(const TString& filepath) 
    // {
    //     std::cout << "Starting GLTF load..." << std::endl;

    // }

    // void VyGLTFModel::Builder::computeTangents() 
    // {
    //     for (size_t i = 0; i < Indices.size(); i += 3) 
    //     {
    //         VyVertex& v0 = Vertices[Indices[i    ]];
    //         VyVertex& v1 = Vertices[Indices[i + 1]];
    //         VyVertex& v2 = Vertices[Indices[i + 2]];

    //         // Edges of the triangle : position delta
    //         Vec3 edge1    = v1.Position - v0.Position;
    //         Vec3 edge2    = v2.Position - v0.Position;

    //         // UV delta
    //         Vec2 deltaUV1 = v1.UV - v0.UV;
    //         Vec2 deltaUV2 = v2.UV - v0.UV;

    //         float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            
    //         if (std::isfinite( f )) 
    //         {
    //             Vec3 tangent;
    //             tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    //             tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    //             tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    //             tangent = glm::normalize(tangent);

    //             Vec3 bitangent;
    //             bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    //             bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    //             bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    //             bitangent = glm::normalize(bitangent);

    //             float handedness = (glm::dot(glm::cross(v0.Normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;

    //             v0.Tangent = v1.Tangent = v2.Tangent = Vec4( tangent, handedness );
    //         }
    //     }
    // }

    // Helper function to get texture path from glTF, handling both URI and embedded images
    // static TString getTexturePath(const tinygltf::Model& inModel, int textureIndex, const TString& baseDir, const TString& cacheDir)
    // {
    //     if (textureIndex < 0 || textureIndex >= static_cast<int>(inModel.textures.size()))
    //     {
    //         return "";
    //     }

    //     const tinygltf::Texture& gltfTexture = inModel.textures[ textureIndex ];

    //     if (gltfTexture.source < 0 || gltfTexture.source >= static_cast<int>(inModel.images.size()))
    //     {
    //         return "";
    //     }

    //     const tinygltf::Image& image = inModel.images[ gltfTexture.source ];

    //     // If image has a URI, it's an external file.
    //     if (!image.uri.empty())
    //     {
    //         // Check if it's a data URI (base64 embedded)
    //         if (image.uri.find("data:") == 0)
    //         {
    //             // It's a data URI - tinygltf has already decoded it into image.image
    //             // We need to write it to a cache file
    //             TString extension = ".png"; // Default to PNG
    //             if (image.mimeType == "image/jpeg")
    //             {
    //                 extension = ".jpg";
    //             }
    //             else if (image.mimeType == "image/png")
    //             {
    //                 extension = ".png";
    //             }

    //             TString cachePath = cacheDir + "/texture_" + std::to_string(gltfTexture.source) + extension;

    //             // Create cache directory if it doesn't exist
    //             std::filesystem::create_directories(cacheDir);

    //             // Write the image data to file
    //             std::ofstream outFile(cachePath, std::ios::binary);
    //             if (outFile.is_open())
    //             {
    //                 outFile.write(reinterpret_cast<const char*>(image.image.data()), image.image.size());
    //                 outFile.close();

    //                 return cachePath;
    //             }
    //             else
    //             {
    //                 std::cerr << YELLOW << "[GLTFImporter] Warning: Failed to write cached texture: " << cachePath << RESET << std::endl;
    //                 return "";
    //             }
    //         }
    //         else
    //         {
    //             // Regular file URI - return path relative to base directory
    //             return baseDir + image.uri;
    //         }
    //     }

    //     // Image is embedded in a bufferView
    //     else if (image.bufferView >= 0)
    //     {
    //         // Image data is embedded in the glTF file
    //         // tinygltf has already loaded it into image.image
    //         TString extension = ".png"; // Default to PNG
    //         if (image.mimeType == "image/jpeg")
    //         {
    //             extension = ".jpg";
    //         }
    //         else if (image.mimeType == "image/png")
    //         {
    //             extension = ".png";
    //         }

    //         TString cachePath = cacheDir + "/embedded_texture_" + std::to_string(gltfTexture.source) + extension;

    //         // Create cache directory if it doesn't exist
    //         std::filesystem::create_directories(cacheDir);

    //         // Write the image data to file
    //         std::ofstream outFile(cachePath, std::ios::binary);
    //         if (outFile.is_open())
    //         {
    //             outFile.write(reinterpret_cast<const char*>(image.image.data()), image.image.size());
    //             outFile.close();

    //             return cachePath;
    //         }
    //         else
    //         {
    //             std::cerr << YELLOW << "[GLTFImporter] Warning: Failed to write embedded texture: " << cachePath << RESET << std::endl;
    //             return "";
    //         }
    //     }

    //     return "";
    // }

    // bool GLTFImporter::load(VyGLTFModel::Builder& builder, const TString& filepath, bool bFlipX, bool bFlipY, bool bFlipZ)
    // {
    //     tinygltf::Model    gltfModel;
    //     tinygltf::TinyGLTF loader;
    //     TString            err;
    //     TString            warn;

    //     // Determine file type and load
    //     bool ret = false;

    //     if (filepath.find(".glb") != TString::npos)
    //     {
    //         ret = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filepath);
    //     }
    //     else
    //     {
    //         ret = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filepath);
    //     }

    //     if (!warn.empty())
    //     {
    //         std::cout << YELLOW << "[GLTFImporter] Warning: " << RESET << warn << std::endl;
    //     }

    //     if (!err.empty())
    //     {
    //         std::cerr << RED << "[GLTFImporter] Error: " << RESET << err << std::endl;
    //         return false;
    //     }

    //     if (!ret)
    //     {
    //         std::cerr << RED << "[GLTFImporter] Failed to load glTF file: " << RESET << filepath << std::endl;
    //         return false;
    //     }

    //     std::cout << "[" << GREEN << "GLTFImporter" << RESET << "]: File loaded successfully" << std::endl;

    //     // Get base directory for texture paths
    //     TString baseDir  = filepath.substr(0, filepath.find_last_of("/\\") + 1);
    //     TString cacheDir = baseDir + ".gltf_texture_cache";

    //     // Flip multipliers
    //     float xMultiplier = bFlipX ? -1.0f : 1.0f;
    //     float yMultiplier = bFlipY ? -1.0f : 1.0f;
    //     float zMultiplier = bFlipZ ? -1.0f : 1.0f;

    //     builder.Vertices  .clear();
    //     builder.Indices   .clear();
    //     builder.Materials .clear();
    //     builder.Primitives.clear();

    //     // Track vertex offsets and counts for each mesh primitive (for morph targets)
    //     // Key: "meshIndex_primitiveIndex", Value: vertex offset/count in builder.Vertices
    //     THashMap<TString, U32> primitiveVertexOffsets;
    //     THashMap<TString, U32> primitiveVertexCounts;

    //     // Map from builder vertex index to original glTF position index (for morph targets)
    //     THashMap<U32, U32> vertexToPositionIndex;

    //     // Load materials first
    //     for (size_t i = 0; i < gltfModel.materials.size(); i++)
    //     {
    //         const auto& gltfMat = gltfModel.materials[ i ];

    //         VyGLTFModel::MaterialInfo matInfo;
    //         matInfo.Name       = gltfMat.name;
    //         matInfo.MaterialId = static_cast<int>(i);

    //         // glTF uses PBR metallic-roughness workflow
    //         const auto& pbr = gltfMat.pbrMetallicRoughness;

    //         // Double Sided
    //         matInfo.PbrMaterial.DoubleSided = gltfMat.doubleSided;

    //         // Alpha Mode
    //         if (gltfMat.alphaMode == "MASK")
    //         {
    //             matInfo.PbrMaterial.AlphaMode   = VyAlphaMode::Mask;
    //             matInfo.PbrMaterial.AlphaCutoff = static_cast<float>(gltfMat.alphaCutoff);
    //         }
    //         else if (gltfMat.alphaMode == "BLEND")
    //         {
    //             matInfo.PbrMaterial.AlphaMode = VyAlphaMode::Blend;
    //         }
    //         else {
    //             matInfo.PbrMaterial.AlphaMode = VyAlphaMode::Opaque;
    //         }

    //         // Extract texture paths (handles both external URIs and embedded images)

    //         // Base color (albedo / diffuse)
    //         if (pbr.baseColorTexture.index >= 0)
    //         {
    //             matInfo.DiffuseTexPath = getTexturePath(gltfModel, pbr.baseColorTexture.index, baseDir, cacheDir);
    //         }

    //         matInfo.PbrMaterial.AlbedoFactor = Vec4(
    //             pbr.baseColorFactor[ 0 ], 
    //             pbr.baseColorFactor[ 1 ], 
    //             pbr.baseColorFactor[ 2 ], 
    //             pbr.baseColorFactor[ 3 ]
    //         );

    //         // Normal
    //         if (gltfMat.normalTexture.index >= 0)
    //         {
    //             matInfo.NormalTexPath = getTexturePath(gltfModel, gltfMat.normalTexture.index, baseDir, cacheDir);
    //         }

    //         // Metallic and Roughness
    //         if (pbr.metallicRoughnessTexture.index >= 0)
    //         {
    //             matInfo.RoughnessTexPath = getTexturePath(gltfModel, pbr.metallicRoughnessTexture.index, baseDir, cacheDir);
                
    //             matInfo.PbrMaterial.UseMetallicRoughnessTexture = true;

    //             if (gltfMat.occlusionTexture.index == pbr.metallicRoughnessTexture.index)
    //             {
    //                 matInfo.PbrMaterial.UseOcclusionRoughnessMetallicTexture = true;
    //             }
    //         }

    //         matInfo.PbrMaterial.MetallicFactor  = static_cast<float>(pbr.metallicFactor);
    //         matInfo.PbrMaterial.RoughnessFactor = static_cast<float>(pbr.roughnessFactor);


    //         // Occlusion
    //         if (gltfMat.occlusionTexture.index >= 0)
    //         {
    //             matInfo.AOTexPath = getTexturePath(gltfModel, gltfMat.occlusionTexture.index, baseDir, cacheDir);
    //         }

    //         matInfo.PbrMaterial.AOFactor = 1.0f;

    //         // Emission
    //         if (gltfMat.emissiveTexture.index >= 0)
    //         {
    //             matInfo.EmissiveTexPath = getTexturePath(gltfModel, gltfMat.emissiveTexture.index, baseDir, cacheDir);
    //         }

    //         matInfo.PbrMaterial.EmissiveFactor = Vec3(
    //             gltfMat.emissiveFactor[ 0 ], 
    //             gltfMat.emissiveFactor[ 1 ], 
    //             gltfMat.emissiveFactor[ 2 ]
    //         );

    //         // Specular Glossiness Workflow
    //         if (gltfMat.extensions.find("KHR_materials_pbrSpecularGlossiness") != gltfMat.extensions.end())
    //         {
    //             const auto& ext = gltfMat.extensions.at("KHR_materials_pbrSpecularGlossiness");

    //             matInfo.PbrMaterial.UseSpecularGlossinessWorkflow = true;

    //             if (ext.Has("diffuseFactor"))
    //             {
    //                 const auto& f = ext.Get("diffuseFactor");

    //                 matInfo.PbrMaterial.AlbedoFactor = Vec4(
    //                     f.Get(0).GetNumberAsDouble(), 
    //                     f.Get(1).GetNumberAsDouble(), 
    //                     f.Get(2).GetNumberAsDouble(), 
    //                     f.Get(3).GetNumberAsDouble()
    //                 );
    //             }

    //             if (ext.Has("specularFactor"))
    //             {
    //                 const auto& f = ext.Get("specularFactor");

    //                 matInfo.PbrMaterial.SpecularFactor = Vec3(
    //                     f.Get(0).GetNumberAsDouble(), 
    //                     f.Get(1).GetNumberAsDouble(), 
    //                     f.Get(2).GetNumberAsDouble()
    //                 );
    //             }

    //             if (ext.Has("glossinessFactor"))
    //             {
    //                 matInfo.PbrMaterial.GlossinessFactor = static_cast<float>(ext.Get("glossinessFactor").GetNumberAsDouble());
    //             }

    //             if (ext.Has("diffuseTexture"))
    //             {
    //                 const auto& tex   = ext.Get("diffuseTexture");
    //                 int         index = tex.Get("index").GetNumberAsInt();

    //                 matInfo.DiffuseTexPath = getTexturePath(gltfModel, index, baseDir, cacheDir);
    //             }

    //             if (ext.Has("specularGlossinessTexture"))
    //             {
    //                 const auto& tex   = ext.Get("specularGlossinessTexture");
    //                 int         index = tex.Get("index").GetNumberAsInt();
                    
    //                 matInfo.SpecularGlossinessTexPath = getTexturePath(gltfModel, index, baseDir, cacheDir);
    //             }
    //         }


    //         // Parse Extensions
    //         // Emissive Strength
    //         if (gltfMat.extensions.find("KHR_materials_emissive_strength") != gltfMat.extensions.end())
    //         {
    //             const auto& ext = gltfMat.extensions.at("KHR_materials_emissive_strength");

    //             if (ext.Has("emissiveStrength"))
    //             {
    //                 matInfo.PbrMaterial.EmissiveStrength = static_cast<float>(ext.Get("emissiveStrength").GetNumberAsDouble());
    //             }
    //         }

    //         // Transmission
    //         if (gltfMat.extensions.find("KHR_materials_transmission") != gltfMat.extensions.end())
    //         {
    //             const auto& ext = gltfMat.extensions.at("KHR_materials_transmission");

    //             if (ext.Has("transmissionFactor"))
    //             {
    //                 matInfo.PbrMaterial.Transmission = static_cast<float>(ext.Get("transmissionFactor").GetNumberAsDouble());
    //             }

    //             if (ext.Has("transmissionTexture"))
    //             {
    //                 const auto& tex   = ext.Get("transmissionTexture");
    //                 int         index = tex.Get("index").GetNumberAsInt();

    //                 matInfo.TransmissionTexPath = getTexturePath(gltfModel, index, baseDir, cacheDir);
    //             }
    //         }

    //         // IOR
    //         if (gltfMat.extensions.find("KHR_materials_ior") != gltfMat.extensions.end())
    //         {
    //             const auto& ext = gltfMat.extensions.at("KHR_materials_ior");

    //             if (ext.Has("ior"))
    //             {
    //                 matInfo.PbrMaterial.IOR = static_cast<float>(ext.Get("ior").GetNumberAsDouble());
    //             }
    //         }

    //         // Iridescence
    //         if (gltfMat.extensions.find("KHR_materials_iridescence") != gltfMat.extensions.end())
    //         {
    //             const auto& ext = gltfMat.extensions.at("KHR_materials_iridescence");

    //             if (ext.Has("iridescenceFactor"))
    //             {
    //                 matInfo.PbrMaterial.Iridescence = static_cast<float>(ext.Get("iridescenceFactor").GetNumberAsDouble());
    //             }

    //             if (ext.Has("iridescenceIor"))
    //             {
    //                 matInfo.PbrMaterial.IridescenceIOR = static_cast<float>(ext.Get("iridescenceIor").GetNumberAsDouble());
    //             }

    //             if (ext.Has("iridescenceThicknessMaximum"))
    //             {
    //                 matInfo.PbrMaterial.IridescenceThickness = static_cast<float>(ext.Get("iridescenceThicknessMaximum").GetNumberAsDouble());
    //             }
    //         }

    //         // Clearcoat
    //         if (gltfMat.extensions.find("KHR_materials_clearcoat") != gltfMat.extensions.end())
    //         {
    //             const auto& ext = gltfMat.extensions.at("KHR_materials_clearcoat");

    //             if (ext.Has("clearcoatFactor"))
    //             {
    //                 matInfo.PbrMaterial.Clearcoat = static_cast<float>(ext.Get("clearcoatFactor").GetNumberAsDouble());
    //             }
                
    //             if (ext.Has("clearcoatRoughnessFactor"))
    //             {
    //                 matInfo.PbrMaterial.ClearcoatRoughness = static_cast<float>(ext.Get("clearcoatRoughnessFactor").GetNumberAsDouble());
    //             }

    //             if (ext.Has("clearcoatTexture"))
    //             {
    //                 const auto& tex   = ext.Get("clearcoatTexture");
    //                 int         index = tex.Get("index").GetNumberAsInt();

    //                 matInfo.ClearcoatTexPath = getTexturePath(gltfModel, index, baseDir, cacheDir);
    //             }

    //             if (ext.Has("clearcoatRoughnessTexture"))
    //             {
    //                 const auto& tex   = ext.Get("clearcoatRoughnessTexture");
    //                 int         index = tex.Get("index").GetNumberAsInt();

    //                 matInfo.ClearcoatRoughnessTexPath = getTexturePath(gltfModel, index, baseDir, cacheDir);
    //             }
    //             if (ext.Has("clearcoatNormalTexture"))
    //             {
    //                 const auto& tex   = ext.Get("clearcoatNormalTexture");
    //                 int         index = tex.Get("index").GetNumberAsInt();

    //                 matInfo.ClearcoatNormalTexPath = getTexturePath(gltfModel, index, baseDir, cacheDir);
    //             }
    //         }

    //         // Volume
    //         if (gltfMat.extensions.find("KHR_materials_volume") != gltfMat.extensions.end())
    //         {
    //             const auto& ext = gltfMat.extensions.at("KHR_materials_volume");

    //             if (ext.Has("thicknessFactor"))
    //             {
    //                 matInfo.PbrMaterial.Thickness = static_cast<float>(ext.Get("thicknessFactor").GetNumberAsDouble());
    //             }

    //             if (ext.Has("attenuationDistance"))
    //             {
    //                 matInfo.PbrMaterial.AttenuationDistance = static_cast<float>(ext.Get("attenuationDistance").GetNumberAsDouble());
    //             }

    //             if (ext.Has("attenuationColor"))
    //             {
    //                 const auto& f = ext.Get("attenuationColor");
                    
    //                 matInfo.PbrMaterial.AttenuationColor = Vec3(f.Get(0).GetNumberAsDouble(), f.Get(1).GetNumberAsDouble(), f.Get(2).GetNumberAsDouble());
    //             }
    //         }

    //         // Texture Transform (KHR_texture_transform)
    //         // We currently only support a single global UV scale, so we check textures in priority order
    //         const tinygltf::ExtensionMap* pTextureExtensions = nullptr;

    //         if (gltfMat.normalTexture.index >= 0 && gltfMat.normalTexture.extensions.count("KHR_texture_transform"))
    //         {
    //             pTextureExtensions = &gltfMat.normalTexture.extensions;
    //         }
    //         else if (pbr.baseColorTexture.index >= 0 && pbr.baseColorTexture.extensions.count("KHR_texture_transform"))
    //         {
    //             pTextureExtensions = &pbr.baseColorTexture.extensions;
    //         }
    //         else if (pbr.metallicRoughnessTexture.index >= 0 && pbr.metallicRoughnessTexture.extensions.count("KHR_texture_transform"))
    //         {
    //             pTextureExtensions = &pbr.metallicRoughnessTexture.extensions;
    //         }
    //         else if (gltfMat.occlusionTexture.index >= 0 && gltfMat.occlusionTexture.extensions.count("KHR_texture_transform"))
    //         {
    //             pTextureExtensions = &gltfMat.occlusionTexture.extensions;
    //         }

    //         if (pTextureExtensions)
    //         {
    //             const auto& ext = pTextureExtensions->at("KHR_texture_transform");

    //             if (ext.Has("scale"))
    //             {
    //                 const auto& scale = ext.Get("scale");

    //                 if (scale.IsArray() && scale.ArrayLen() >= 1)
    //                 {
    //                     // Use the X scale as the uniform scale
    //                     matInfo.PbrMaterial.UVScale = static_cast<float>(scale.Get(0).GetNumberAsDouble());
    //                 }
    //             }
    //         }

    //         builder.Materials.push_back( matInfo );

    //         TString alphaModeStr = "OPAQUE";

    //         if (matInfo.PbrMaterial.AlphaMode == VyAlphaMode::Mask)
    //         {
    //             alphaModeStr = "MASK";
    //         }
    //         else if (matInfo.PbrMaterial.AlphaMode == VyAlphaMode::Blend)
    //         {
    //             alphaModeStr = "BLEND";
    //         }

    //         std::cout 
    //             << "[" << GREEN << " Material " << RESET << "] " 
    //             << BLUE << matInfo.Name << RESET 
    //             << " -> PBR("
    //                 << "albedo=" 
    //                     << matInfo.PbrMaterial.AlbedoFactor.r << ","
    //                     << matInfo.PbrMaterial.AlbedoFactor.g << "," 
    //                     << matInfo.PbrMaterial.AlbedoFactor.b 
    //                 << ", metallic="  << matInfo.PbrMaterial.MetallicFactor
    //                 << ", roughness=" << matInfo.PbrMaterial.RoughnessFactor 
    //                 << ", alphaMode=" << alphaModeStr 
    //             << ")" 
    //             << std::endl;

    //     } // Process all meshes in the scene

    //     const tinygltf::Scene& scene = gltfModel.scenes[ gltfModel.defaultScene >= 0 ? gltfModel.defaultScene : 0 ];

    //     // Lambda to process a node recursively
    //     std::function<void(int)> processNode = [&](int nodeIndex) 
    //     {
    //         const tinygltf::Node& node = gltfModel.nodes[ nodeIndex ];

    //         U32 vertexOffset = 0;
    //         U32 indexOffset  = 0;

    //         // Process mesh if present
    //         if (node.mesh >= 0)
    //         {
    //             int meshIndex = node.mesh;
                
    //             const tinygltf::Mesh& mesh = gltfModel.meshes[ meshIndex ];
                
    //             for (size_t primIndex = 0; primIndex < mesh.primitives.size(); primIndex++)
    //             {
    //                 const auto& gltfPrimitive = mesh.primitives[ primIndex ];

    //                 U32 vertexCount = 0;
    //                 U32 indexCount  = 0;

    //                 bool bHasIndices = gltfPrimitive.indices > -1;
    //                 int  materialId  = gltfPrimitive.material; // Get accessors for vertex attributes

    //                 // Buffer pointers & data strides.
    //                 const float* pPosBuffer      = nullptr;
    //                 const float* pNormalBuffer   = nullptr;
    //                 const float* pTangentBuffer  = nullptr;
    //                 const float* pUV0Buffer      = nullptr;
    //                 const float* pColor0Buffer   = nullptr;
                    
    //                 int posByteStride;
    //                 int normByteStride;
    //                 int tangentByteStride;
    //                 int uv0ByteStride;
    //                 int color0ByteStride;

    //                 // [ Position Attributes (REQUIRED) ]
    //                 auto positionEntry = gltfPrimitive.attributes.find("POSITION");
    //                 if(positionEntry != gltfPrimitive.attributes.end())
    //                 {
    //                     const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(positionEntry->second)];
    //                     const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)  ];
    //                     const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)    ];
                        
    //                     pPosBuffer = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
                        
    //                     posByteStride = accessor.ByteStride(bufferView) 
    //                         ? (accessor.ByteStride(bufferView) / sizeof(float)) 
    //                         : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

    //                     // Set vertex count.
    //                     vertexCount = static_cast<U32>(accessor.count);
    //                 }
    //                 else {
    //                     VY_THROW_RUNTIME_ERROR("No Position attributes found.");
    //                 }

    //                 // [ Normal Attributes (optional) ]
    //                 auto normEntry = gltfPrimitive.attributes.find("NORMAL");
    //                 if(normEntry != gltfPrimitive.attributes.end())
    //                 {
    //                     const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(normEntry->second)  ];
    //                     const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)];
    //                     const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)  ];

    //                     pNormalBuffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));

    //                     normByteStride = accessor.ByteStride(bufferView) 
    //                         ? (accessor.ByteStride(bufferView) / sizeof(float)) 
    //                         : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3); 
    //                 }

    //                 // [ Tangent Attributes (optional) ]
    //                 auto tangentEntry = gltfPrimitive.attributes.find("TANGENT");
    //                 if (tangentEntry != gltfPrimitive.attributes.end()) 
    //                 {
    //                     const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(tangentEntry->second)];
    //                     const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView) ];
    //                     const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)   ];

    //                     pTangentBuffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                    
    //                     tangentByteStride = accessor.ByteStride(bufferView) 
    //                         ? (accessor.ByteStride(bufferView) / sizeof(float)) 
    //                         : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
    //                 }

    //                 // [ UV 0 Attributes (optional) ]
    //                 auto uvEntry0 = gltfPrimitive.attributes.find("TEXCOORD_0");
    //                 if (uvEntry0 != gltfPrimitive.attributes.end()) 
    //                 {
    //                     const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(uvEntry0->second)   ];
    //                     const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)];
    //                     const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)  ];

    //                     pUV0Buffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                    
    //                     uv0ByteStride = accessor.ByteStride(bufferView) 
    //                         ? (accessor.ByteStride(bufferView) / sizeof(float)) 
    //                         : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2); 
    //                 }

    //                 // [ Color 0 Attributes (optional) ]
    //                 auto colorEntry = gltfPrimitive.attributes.find("COLOR_0");
    //                 if (colorEntry != gltfPrimitive.attributes.end()) 
    //                 {
    //                     const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(colorEntry->second) ];
    //                     const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)];
    //                     const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)  ];

    //                     pColor0Buffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                    
    //                     color0ByteStride = accessor.ByteStride(bufferView) 
    //                         ? (accessor.ByteStride(bufferView) / sizeof(float)) 
    //                         : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3); 
    //                 }
                    
    //                 // [ VERTICES ]
    //                 for (U32 vIndex = 0; vIndex < vertexCount; vIndex++)
    //                 {
    //                     VyVertex v{};
                        
    //                     v.Position =                                 glm::make_vec3(&pPosBuffer    [vIndex * posByteStride    ]                );
    //                     v.Normal   = glm::normalize(pNormalBuffer  ? glm::make_vec3(&pNormalBuffer [vIndex * normByteStride   ]) : Vec3{ 0.0f });
    //                     v.Tangent  = glm::normalize(pTangentBuffer ? glm::make_vec4(&pTangentBuffer[vIndex * tangentByteStride]) : Vec4{ 0.0f });
    //                     v.UV       = pUV0Buffer                    ? glm::make_vec2(&pUV0Buffer    [vIndex * uv0ByteStride    ]) : Vec2{ 0.0f };
    //                     v.Color    = pColor0Buffer                 ? glm::make_vec3(&pColor0Buffer [vIndex * color0ByteStride ]) : Vec3{ 1.0f };

    //                     builder.Vertices.push_back( v );
    //                 }

    //                 // [ INDICES ]
    //                 if (bHasIndices) 
    //                 {
    //                     const auto& indexAccessor   = gltfModel.accessors  [gltfPrimitive.indices > -1 ? gltfPrimitive.indices : 0];
    //                     const auto& indexBufferView = gltfModel.bufferViews[static_cast<size_t>(indexAccessor.bufferView)         ];
    //                     const auto& indexBuffer     = gltfModel.buffers    [static_cast<size_t>(indexBufferView.buffer)           ];
                        
    //                     // Set index count.
    //                     indexCount = static_cast<U32>( indexAccessor.count );

    //                     // Raw index data to cast.
    //                     const void* pIndexData = &(indexBuffer.data[indexAccessor.byteOffset + indexBufferView.byteOffset]);

    //                     // TODO: Raw indexing of the indices array would be considerably faster for index buffer here than push_back.
    //                     switch (indexAccessor.componentType) 
    //                     {
    //                         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: 
    //                         {
    //                             const U32* pCastData = static_cast<const U32*>(pIndexData);

    //                             for (U32 i = 0; i < indexCount; i++) 
    //                             {
    //                                 builder.Indices.push_back( pCastData[ i ] );
    //                             }

    //                         } break;
                            
    //                         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: 
    //                         {
    //                             const U16* pCastData = static_cast<const U16*>(pIndexData);

    //                             for (U32 i = 0; i < indexCount; i++) 
    //                             {
    //                                 builder.Indices.push_back( static_cast<U32>(pCastData[ i ]) );
    //                             }
                                
    //                         } break;
                            
    //                         case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: 
    //                         {
    //                             const U8* pCastData = static_cast<const U8*>(pIndexData);

    //                             for (U32 i = 0; i < indexCount; i++) 
    //                             {
    //                                 builder.Indices.push_back( static_cast<U32>(pCastData[ i ]) );
    //                             }
                                
    //                         } break;
                            
    //                         default:
    //                         {
    //                             VY_THROW_RUNTIME_ERROR("Attempted to load model Indices with an unsupported data type.");
    //                         }
    //                     }
    //                 }
                    
    //                 // std::cout << "[GLTFImporter] Mesh " << meshIndex << " prim " << primIndex << " added " << primitiveVertexCount << " vertices" << std::endl;
    //             }
    //         }

    //         // Process children recursively
    //         for (int childIndex : node.children)
    //         {
    //             processNode( childIndex );
    //         }
    //     };

    //     // Process all root nodes
    //     for (int nodeIndex : scene.nodes)
    //     {
    //         processNode( nodeIndex );
    //     }

    //     // Create sub-meshes from grouped indices
    //     U32 currentOffset = 0;

    //     for (auto& [ matId, matIndices ] : indicesByMaterial)
    //     {
    //         if (!matIndices.empty())
    //         {
    //             VyGLTFModel::VyPrimitive subMesh{};
    //             {
    //                 subMesh.MaterialId  = matId;
    //                 subMesh.IndexOffset = currentOffset;
    //                 subMesh.IndexCount  = static_cast<U32>(matIndices.size());
    //             }

    //             builder.Primitives.push_back( subMesh );

    //             currentOffset += subMesh.IndexCount;
    //         }
    //     }
    // }

    // void VyGLTFModel::Builder::loadModelFromGLTF(const TString& filepath, bool bFlipX, bool bFlipY, bool bFlipZ)
    // {

    // }

    VyGLTFModel::VyGLTFModel(const TString& filepath, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool)
    {
        m_DefaultWhite  = VyTexture::createWhiteTexture(); 
        m_DefaultNormal = VyTexture::createNormalTexture();
    
        tinygltf::Model    gltfModel;
        tinygltf::TinyGLTF loader;
        TString            err;
        TString            warn;

        // Load Model
        if (filepath.find(".glb") != TString::npos)
        {
            if (!loader.LoadBinaryFromFile( &gltfModel, &err, &warn, filepath ))
            {
                throw std::runtime_error("Failed to load glb file!");
            }
        }
        if (filepath.find(".gltf") != TString::npos)
        {
            if (!loader.LoadASCIIFromFile( &gltfModel, &err, &warn, filepath ))
            {
                throw std::runtime_error("Failed to load gltf file!");
            }
        }

        if (!warn.empty())
        {
            std::cout << YELLOW << "[GLTFImporter] Warning: " << RESET << warn << std::endl;
        }

        if (!err.empty())
        {
            std::cerr << RED << "[GLTFImporter] Error: " << RESET << err << std::endl;
        }

        ModelLoadInfo info{};
        info.Name = Utils::filenameFromPath( filepath );

        auto path = TPath( filepath );

        for (auto& texture : gltfModel.images)
        {
            TString texPath = path.parent_path().append(texture.uri).generic_string();
            m_Textures.push_back( VyTexture::createFromFilepath( texPath ));

            // std::cout << "TexCount: " << info.TextureCount << ", Tex: " << texPath << std::endl;
            info.TextureCount++;
        }

        for (auto& scene : gltfModel.scenes)
        {
            for (size_t i = 0; i < scene.nodes.size(); i++)
            {
                auto& node = gltfModel.nodes[ i ];

                U32 vertexOffset = 0;
                U32 indexOffset  = 0;

                for (auto& gltfPrimitive : gltfModel.meshes[ node.mesh ].primitives)
                {
                    U32 vertexCount = 0;
                    U32 indexCount  = 0;

                    bool bHasIndices = gltfPrimitive.indices > -1;

                    // Buffer pointers & data strides.
                    const float* pPosBuffer      = nullptr;
                    const float* pNormalBuffer   = nullptr;
                    const float* pTangentBuffer  = nullptr;
                    const float* pUV0Buffer      = nullptr;
                    const float* pColor0Buffer   = nullptr;
                    
                    int posByteStride;
                    int normByteStride;
                    int tangentByteStride;
                    int uv0ByteStride;
                    int color0ByteStride;

                    // [ Position Attributes (REQUIRED) ]
                    auto positionEntry = gltfPrimitive.attributes.find("POSITION");
                    if(positionEntry != gltfPrimitive.attributes.end())
                    {
                        const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(positionEntry->second)];
                        const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)  ];
                        const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)    ];
                        
                        pPosBuffer = reinterpret_cast<const float*>(&(buffer.data[accessor.byteOffset + bufferView.byteOffset]));
                        
                        posByteStride = accessor.ByteStride(bufferView) 
                            ? (accessor.ByteStride(bufferView) / sizeof(float)) 
                            : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

                        // Set vertex count.
                        vertexCount = static_cast<U32>(accessor.count);
                    }
                    else {
                        VY_THROW_RUNTIME_ERROR("No Position attributes found.");
                    }

                    // [ Normal Attributes (optional) ]
                    auto normEntry = gltfPrimitive.attributes.find("NORMAL");
                    if(normEntry != gltfPrimitive.attributes.end())
                    {
                        const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(normEntry->second)  ];
                        const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)];
                        const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)  ];

                        pNormalBuffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));

                        normByteStride = accessor.ByteStride(bufferView) 
                            ? (accessor.ByteStride(bufferView) / sizeof(float)) 
                            : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3); 
                    }

                    // [ Tangent Attributes (optional) ]
                    auto tangentEntry = gltfPrimitive.attributes.find("TANGENT");
                    if (tangentEntry != gltfPrimitive.attributes.end()) 
                    {
                        const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(tangentEntry->second)];
                        const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView) ];
                        const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)   ];

                        pTangentBuffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                    
                        tangentByteStride = accessor.ByteStride(bufferView) 
                            ? (accessor.ByteStride(bufferView) / sizeof(float)) 
                            : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
                    }

                    // [ UV 0 Attributes (optional) ]
                    auto uvEntry0 = gltfPrimitive.attributes.find("TEXCOORD_0");
                    if (uvEntry0 != gltfPrimitive.attributes.end()) 
                    {
                        const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(uvEntry0->second)   ];
                        const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)];
                        const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)  ];

                        pUV0Buffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                    
                        uv0ByteStride = accessor.ByteStride(bufferView) 
                            ? (accessor.ByteStride(bufferView) / sizeof(float)) 
                            : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2); 
                    }

                    // [ Color 0 Attributes (optional) ]
                    auto colorEntry = gltfPrimitive.attributes.find("COLOR_0");
                    if (colorEntry != gltfPrimitive.attributes.end()) 
                    {
                        const auto& accessor   = gltfModel.accessors  [static_cast<size_t>(colorEntry->second) ];
                        const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(accessor.bufferView)];
                        const auto& buffer     = gltfModel.buffers    [static_cast<size_t>(bufferView.buffer)  ];

                        pColor0Buffer = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));
                    
                        color0ByteStride = accessor.ByteStride(bufferView) 
                            ? (accessor.ByteStride(bufferView) / sizeof(float)) 
                            : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3); 
                    }

                    // [ Populate Vertex Data ]
                    for (U32 vIndex = 0; vIndex < vertexCount; vIndex++)
                    {
                        VyVertex v{};
                        
                        v.Position =                                 glm::make_vec3(&pPosBuffer    [vIndex * posByteStride    ]                );
                        v.Normal   = glm::normalize(pNormalBuffer  ? glm::make_vec3(&pNormalBuffer [vIndex * normByteStride   ]) : Vec3{ 0.0f });
                        v.Tangent  = glm::normalize(pTangentBuffer ? glm::make_vec4(&pTangentBuffer[vIndex * tangentByteStride]) : Vec4{ 0.0f });
                        v.UV       = pUV0Buffer                    ? glm::make_vec2(&pUV0Buffer    [vIndex * uv0ByteStride    ]) : Vec2{ 0.0f };
                        v.Color    = pColor0Buffer                 ? glm::make_vec3(&pColor0Buffer [vIndex * color0ByteStride ]) : Vec3{ 1.0f };

                        m_Vertices.push_back( v );
                    }

                    // [ INDICES ]
                    {
                        if (bHasIndices) 
                        {
                            const auto& indexAccessor   = gltfModel.accessors  [gltfPrimitive.indices > -1 ? gltfPrimitive.indices : 0];
                            const auto& indexBufferView = gltfModel.bufferViews[static_cast<size_t>(indexAccessor.bufferView)         ];
                            const auto& indexBuffer     = gltfModel.buffers    [static_cast<size_t>(indexBufferView.buffer)           ];
                            
                            // Set index count.
                            indexCount = static_cast<U32>( indexAccessor.count );

                            // Raw index data to cast.
                            const void* pIndexData = &(indexBuffer.data[indexAccessor.byteOffset + indexBufferView.byteOffset]);

                            // TODO: Raw indexing of the indices array would be considerably faster for index buffer here than push_back.
                            switch (indexAccessor.componentType) 
                            {
                                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: 
                                {
                                    const U32* pCastData = static_cast<const U32*>(pIndexData);

                                    for (U32 i = 0; i < indexCount; i++) 
                                    {
                                        m_Indices.push_back( pCastData[ i ] );
                                    }

                                } break;
                                
                                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: 
                                {
                                    const U16* pCastData = static_cast<const U16*>(pIndexData);

                                    for (U32 i = 0; i < indexCount; i++) 
                                    {
                                        m_Indices.push_back( static_cast<U32>(pCastData[ i ]) );
                                    }
                                    
                                } break;
                                
                                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: 
                                {
                                    const U8* pCastData = static_cast<const U8*>(pIndexData);

                                    for (U32 i = 0; i < indexCount; i++) 
                                    {
                                        m_Indices.push_back( static_cast<U32>(pCastData[ i ]) );
                                    }
                                    
                                } break;
                                
                                default:
                                {
                                    VY_THROW_RUNTIME_ERROR("Attempted to load model Indices with an unsupported data type.");
                                }
                            }
                        }
                    }

                    // [ MATERIALS ]
                    VyPBRMaterial material{};
                    {
                        if (gltfPrimitive.material != -1)
                        {
                            tinygltf::Material& gltfMaterial = gltfModel.materials[ gltfPrimitive.material ];

                            // [ PBR Metallic Roughness Factors ]
                            const auto& pbr = gltfMaterial.pbrMetallicRoughness;

                            // [ PBR - Albedo ]
                            if (pbr.baseColorTexture.index != -1)
                            {
                                U32 textureIndex = pbr.baseColorTexture.index;
                                U32 imageIndex   = gltfModel.textures[textureIndex].source;

                                material.AlbedoMap = m_Textures[ imageIndex ];

                                // std::cout << "[AL] TexIndex: " << textureIndex << ", ImgIndex: " << imageIndex << std::endl;
                                // material.HasAlbedo = 1;
                            }
                            else {
                                material.AlbedoMap = m_DefaultWhite;
                            }

                            // [ PBR - MetallicRoughness ]
                            if (pbr.metallicRoughnessTexture.index != -1)
                            {
                                U32 textureIndex = pbr.metallicRoughnessTexture.index;
                                U32 imageIndex   = gltfModel.textures[textureIndex].source;
                                
                                material.MetallicRoughnessMap        = m_Textures[imageIndex];
                                material.UseMetallicRoughnessTexture = true;

                                // [ ORM ]
                                if (gltfMaterial.occlusionTexture.index == pbr.metallicRoughnessTexture.index)
                                {
                                    material.UseOcclusionRoughnessMetallicTexture = true;
                                }
                            }
                            else {
                                material.MetallicRoughnessMap = m_DefaultNormal;
                            }

                            // [ Occlusion ]
                            if (gltfMaterial.occlusionTexture.index != -1)
                            {
                                U32 textureIndex = gltfMaterial.occlusionTexture.index;
                                U32 imageIndex   = gltfModel.textures[textureIndex].source;

                                material.AOMap = m_Textures[imageIndex];
                            }
                            else {
                                material.AOMap = m_DefaultNormal;
                            }

                            // [ Emissive ]
                            if (gltfMaterial.emissiveTexture.index != -1)
                            {
                                U32 textureIndex = gltfMaterial.emissiveTexture.index;
                                U32 imageIndex   = gltfModel.textures[textureIndex].source;

                                material.EmissiveMap = m_Textures[imageIndex];
                            }
                            else {
                                material.EmissiveMap = m_DefaultWhite;
                            }

                            // [ Normal ]
                            if (gltfMaterial.normalTexture.index != -1)
                            {
                                U32 textureIndex = gltfMaterial.normalTexture.index;
                                U32 imageIndex   = gltfModel.textures[textureIndex].source;
                                
                                material.NormalMap = m_Textures[imageIndex];
                            }
                            else {
                                material.NormalMap = m_DefaultNormal;
                            }
                        }

                        // [ Fallback ]
                        else {
                            material.AlbedoMap            = m_DefaultWhite;
                            material.MetallicRoughnessMap = m_DefaultNormal;
                            material.EmissiveMap          = m_DefaultWhite;
                            material.AOMap                = m_DefaultNormal;
                            material.NormalMap            = m_DefaultNormal;
                        }
                    }

                    createDescriptorSet(material, materialSetLayout, descriptorPool);

                    VyPrimitive primitive{};
                    {
                        primitive.FirstIndex  = indexOffset;
                        primitive.IndexCount  = indexCount;
                        primitive.FirstVertex = vertexOffset;
                        primitive.VertexCount = vertexCount;
                        primitive.Material    = material;
                    }
                    
                    m_Primitives.push_back( primitive );

                    vertexOffset += vertexCount;
                    indexOffset  += indexCount;

                    info.MeshCount++;
                    info.VertexCount = vertexCount;
                    info.IndexCount  = indexCount;

                } // [ End of mesh loop ]

                info.NodeCount++;

            } // [ End of node loop ]

            createVertexBuffer( m_Vertices );
            createIndexBuffer ( m_Indices  );

            info.SceneCount++;

        } // [ End of scene loop ]

		std::stringstream ss;
        ss  << "\n--------------------------------------------------------------------------" << '\n'
			<< "[" << CYAN "VyModel Load Stats" RESET "] " << '\n'
            << " - Name       : " << info.Name             << '\n'
            // << " - Scenes     : " << info.SceneCount       << '\n'
            // << " - Nodes      : " << info.NodeCount        << '\n'
			<< " - Meshes     : " << info.MeshCount        << '\n'
            << " - Vertices   : " << info.VertexCount      << '\n'
			<< " - Indices    : " << info.IndexCount       << '\n'
            << " - Textures   : " << info.TextureCount     << '\n'
			<< "--------------------------------------------------------------------------"   << '\n'
        ;

        std::cout << ss.str() << std::endl;
    }


    void VyGLTFModel::createDescriptorSet(VyPBRMaterial& material, VyDescriptorSetLayout& materialSetLayout, VyDescriptorPool& descriptorPool)
    {
        VkDescriptorImageInfo albedoInfo   = material.AlbedoMap           ->descriptorImageInfo();
        VkDescriptorImageInfo normalInfo   = material.NormalMap           ->descriptorImageInfo();
        VkDescriptorImageInfo mrInfo       = material.MetallicRoughnessMap->descriptorImageInfo();
        VkDescriptorImageInfo aoInfo       = material.AOMap               ->descriptorImageInfo();
        VkDescriptorImageInfo emissiveInfo = material.EmissiveMap         ->descriptorImageInfo();

        VyDescriptorWriter( materialSetLayout, descriptorPool )
            .writeImage( 0, &albedoInfo   )
            .writeImage( 1, &normalInfo   )
            .writeImage( 2, &mrInfo       )
            .writeImage( 3, &aoInfo       )
            .writeImage( 4, &emissiveInfo )
            .build( material.DescriptorSet );
    }

    // =====================================================================================================================

    VyGLTFModel::~VyGLTFModel()
    {
    }

    // =====================================================================================================================

    void VyGLTFModel::bind(VkCommandBuffer cmdBuffer)
    {
        VkBuffer     buffers[] = { m_VertexBuffer->handle() };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(cmdBuffer, 0, 1, buffers, offsets);
        
        if (m_HasIndexBuffer) 
        {
            vkCmdBindIndexBuffer(cmdBuffer, m_IndexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    // =====================================================================================================================

    void VyGLTFModel::draw(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, int setCount, bool bRenderMaterial)
    {
        for (auto& primitive : m_Primitives)
        {
            if (m_HasIndexBuffer)
            {
                if (bRenderMaterial)
                {
                    TVector<VkDescriptorSet> sets = { primitive.Material.DescriptorSet };

                    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, setCount, sets.size(), sets.data(), 0, nullptr);
                }

                vkCmdDrawIndexed(cmdBuffer, primitive.IndexCount, 1, primitive.FirstIndex, primitive.FirstVertex, 0);
            }
            else
            {
                vkCmdDraw(cmdBuffer, primitive.VertexCount, 1, 0, 0);
            }
        }
    }

    // =====================================================================================================================

    void VyGLTFModel::createVertexBuffer(const TVector<VyVertex>& vertices) 
    {
        U32 vertexCount = static_cast<U32>( vertices.size() );

        VY_ASSERT(vertexCount >= 3, "Vertex count must be at least 3");
        
        U32 vertexSize = sizeof(vertices[0]);

        m_VertexBuffer = MakeUnique<VyBuffer>( VyBuffer::vertexBuffer( "model", vertexSize, vertexCount ) );
        
        m_VertexBuffer->upload( vertices );
    }

    // =====================================================================================================================

    void VyGLTFModel::createIndexBuffer(const TVector<U32>& indices) 
    {
        U32 indexCount   = static_cast<U32>( indices.size() );
        m_HasIndexBuffer = indexCount > 0;
        
        if (!m_HasIndexBuffer) { return; }

        U32 indexSize = sizeof(indices[0]);

        m_IndexBuffer = MakeUnique<VyBuffer>( VyBuffer::indexBuffer( "model", indexSize, indexCount ) );
        
        m_IndexBuffer->upload( indices );
    }

    // =====================================================================================================================

}