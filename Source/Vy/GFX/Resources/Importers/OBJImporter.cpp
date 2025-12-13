#include <Vy/GFX/Resources/Importers/OBJImporter.h>
#include <VyLib/Util/Hash.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// Hash function for Model::Vertex
namespace std 
{
    template <> 
    struct hash<Vy::Model::Vertex>
    {
        size_t operator()(Vy::Model::Vertex const& vertex) const
        {
            size_t seed = 0;
            Vy::Hash::hashCombine(seed, vertex.Position, vertex.Color, vertex.Normal, vertex.UV);
            return seed;
        }
    };
} // namespace std

namespace Vy 
{

    // Helper function for MTL to PBR conversion
    static VyPBRMaterial mtlToPBR(const glm::vec3& Kd, const glm::vec3& Ks, float Ns, String& matName)
    {
        // Convert MTL (Phong) material to PBR material
        // Note: This conversion is heuristic-based. Future improvements:
        // - Transparency: Use MTL 'd' parameter for alpha/transmission (HeadlightGlass, Glass)
        // - Clearcoat: Add second specular layer for car paint (Roughness ~0.05)
        // - Anisotropy: For brushed metals or fabric materials

        VyPBRMaterial pbr;

        // Roughness from Ns
        Ns              = glm::clamp(Ns, 1.0f, 1000.0f);
        float roughness = sqrtf(2.0f / (Ns + 2.0f));
        roughness       = powf(roughness, 0.5f);
        roughness       = glm::clamp(roughness, 0.0f, 1.0f);

        float specularIntensity = glm::clamp((Ks.r + Ks.g + Ks.b) / 3.0f, 0.0f, 1.0f);
        float kdLuminance       = glm::dot(Kd, glm::vec3(0.299f, 0.587f, 0.114f));

        // --- Force metals by name
        String nameLower = matName;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
        bool forceMetal = (nameLower.find("chrome") != String::npos || nameLower.find("mirror") != String::npos ||
                        nameLower.find("aluminum") != String::npos || nameLower.find("metal") != String::npos);

        if (forceMetal || (glm::length(Kd) < 0.05f && specularIntensity > 0.6f))
        {
            pbr.Metallic = 1.0f;
            roughness    = glm::min(roughness, 0.02f); // mirror-like

            // Use specular color for metallic tint (gold, copper, chrome with tint)
            // This preserves colored reflections for metals
            pbr.Albedo = glm::vec4(glm::clamp(Ks, glm::vec3(0.04f), glm::vec3(1.0f)), 1.0f);
        }
        else
        {
        // Non-obvious metals
        float maxKs      = glm::max(glm::max(Ks.r, Ks.g), Ks.b);
        float minKs      = glm::min(glm::min(Ks.r, Ks.g), Ks.b);
        float tintAmount = glm::clamp((maxKs - minKs) / glm::max(maxKs, 1e-6f), 0.0f, 1.0f);

        float metallic = glm::smoothstep(0.05f, 0.25f, tintAmount);
        metallic *= glm::smoothstep(0.05f, 0.0f, kdLuminance);
        pbr.Metallic = glm::clamp(metallic, 0.0f, 1.0f);

        // Blend albedo based on metallic value for partial metals
        pbr.Albedo = glm::vec4(glm::mix(Kd, glm::clamp(Ks, glm::vec3(0.04f), glm::vec3(1.0f)), pbr.Metallic), 1.0f);
        }

        pbr.Roughness = roughness;
        pbr.AO        = 1.0f;

        // --- Detect clearcoat materials (car paint, lacquered surfaces)
        bool isCarPaint = (nameLower.find("bmw") != String::npos || nameLower.find("carshell") != String::npos || nameLower.find("paint") != String::npos);

        if (isCarPaint && pbr.Metallic < 0.5f) // Dielectric paint with clearcoat
        {
            pbr.Clearcoat          = 1.0f;  // Full clearcoat layer
            pbr.ClearcoatRoughness = 0.05f; // Smooth glossy finish
        }

        // --- Detect anisotropic materials (brushed metals)
        bool isBrushedMetal = (nameLower.find("brushed") != String::npos || nameLower.find("aluminum") != String::npos || nameLower.find("steel") != String::npos);

        if (isBrushedMetal)
        {
            pbr.Anisotropic         = 0.8f; // Strong anisotropic effect
            pbr.AnisotropicRotation = 0.0f; // Aligned with tangent
        }

        return pbr;
    }


    bool OBJImporter::load(Model::Builder& builder, const String& filepath, bool flipX, bool flipY, bool flipZ)
    {
        tinyobj::attrib_t                attrib;
        TVector<tinyobj::shape_t>    shapes;
        TVector<tinyobj::material_t> tinyMaterials;
        String                        warn;
        String                        err;

        // Get the directory path for MTL file
        String mtlBaseDir = filepath.substr(0, filepath.find_last_of("/\\") + 1);

        if (!tinyobj::LoadObj(&attrib, &shapes, &tinyMaterials, &warn, &err, filepath.c_str(), mtlBaseDir.c_str()))
        {
            // std::cerr << RED << "[OBJImporter] Error: " << RESET << warn + err << std::endl;
            return false;
        }

    #if defined(DEBUG)
        if (!warn.empty())
        {
            std::cout << YELLOW << "[OBJImporter] Warning: " << RESET << warn << std::endl;
        }
    #endif

        // Convert TinyObj materials to our PBR materials
        builder.Materials.clear();
        for (const auto& mat : tinyMaterials)
        {
            Model::MaterialInfo matInfo;
            matInfo.Name = mat.name;

            // Convert TinyObj material to PBR using helper function
            auto  Kd = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
            auto  Ks = glm::vec3(mat.specular[0], mat.specular[1], mat.specular[2]);
            float Ns = mat.shininess;

            matInfo.PBRMaterial = mtlToPBR(Kd, Ks, Ns, matInfo.Name);
            matInfo.MaterialId  = static_cast<int>(builder.Materials.size());

            // Extract texture paths from MTL file
            matInfo.DiffuseTexPath   = mat.diffuse_texname;
            matInfo.NormalTexPath    = mat.bump_texname.empty() ? mat.normal_texname : mat.bump_texname;
            matInfo.RoughnessTexPath = mat.roughness_texname.empty() ? mat.specular_texname : mat.roughness_texname;
            matInfo.AOTexPath        = mat.ambient_texname;

            builder.Materials.push_back(matInfo);

            std::cout << "[Material] " << mat.name << " -> PBR(albedo=" << matInfo.PBRMaterial.Albedo.r << ","
                        << matInfo.PBRMaterial.Albedo.g << "," << matInfo.PBRMaterial.Albedo.b << ", Metallic=" << matInfo.PBRMaterial.Metallic
                        << ", Roughness=" << matInfo.PBRMaterial.Roughness << ")" << std::endl;
            }

        // Group indices by material to create sub-meshes
        THashMap<int, TVector<U32>>  indicesByMaterial;
        THashMap<Model::Vertex, U32> uniqueVertices{};

        builder.Vertices.clear();
        builder.Indices.clear();

        float xMultiplier = flipX ? -1.0f : 1.0f;
        float yMultiplier = flipY ? -1.0f : 1.0f;
        float zMultiplier = flipZ ? -1.0f : 1.0f;

        for (const auto& shape : shapes)
        {
            for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++)
            {
                // Get material ID for this face
                int MaterialId = shape.mesh.material_ids[f];

                for (size_t v = 0; v < 3; v++)
                {
                    const auto&   index = shape.mesh.indices[3 * f + v];
                    Model::Vertex vertex{};
                    vertex.MaterialId = MaterialId;

                    if (index.vertex_index >= 0)
                    {
                        vertex.Position = {
                            xMultiplier * attrib.vertices[3 * index.vertex_index + 0],
                            yMultiplier * attrib.vertices[3 * index.vertex_index + 1],
                            zMultiplier * attrib.vertices[3 * index.vertex_index + 2],
                        };

                        if (!attrib.colors.empty())
                        {
                            vertex.Color = {
                                attrib.colors[3 * index.vertex_index + 0],
                                attrib.colors[3 * index.vertex_index + 1],
                                attrib.colors[3 * index.vertex_index + 2],
                            };
                        }
                        else
                        {
                            vertex.Color = {1.0f, 1.0f, 1.0f};
                        }
                    }

                    if (index.normal_index >= 0)
                    {
                        vertex.Normal = {
                            xMultiplier * attrib.normals[3 * index.normal_index + 0],
                            yMultiplier * attrib.normals[3 * index.normal_index + 1],
                            zMultiplier * attrib.normals[3 * index.normal_index + 2],
                        };
                    }

                    if (index.texcoord_index >= 0)
                    {
                        vertex.UV = {
                                attrib.texcoords[2 * index.texcoord_index + 0],
                                attrib.texcoords[2 * index.texcoord_index + 1],
                        };
                    }

                    if (uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = static_cast<U32>(builder.Vertices.size());
                        builder.Vertices.push_back(vertex);
                    }

                    U32 vertexIndex = uniqueVertices[vertex];
                    builder.Indices.push_back(vertexIndex);

                    // Group indices by material
                    indicesByMaterial[MaterialId].push_back(vertexIndex);
                }
            }
        }

        // Create sub-meshes from grouped indices
        builder.SubMeshes.clear();
        U32 currentOffset = 0;

        for (auto& [matId, matIndices] : indicesByMaterial)
        {
            if (!matIndices.empty())
            {
                Model::SubMesh subMesh{};
                {
                    subMesh.MaterialId  = matId;
                    subMesh.IndexOffset = currentOffset;
                    subMesh.IndexCount  = static_cast<U32>(matIndices.size());
                }

                builder.SubMeshes.push_back(subMesh);

                currentOffset += subMesh.IndexCount;
            }
        }

        // Rebuild indices array grouped by material
        TVector<U32> groupedIndices;
        groupedIndices.reserve(builder.Indices.size());

        for (const auto& subMesh : builder.SubMeshes)
        {
            const auto& matIndices = indicesByMaterial[subMesh.MaterialId];
            groupedIndices.insert(groupedIndices.end(), matIndices.begin(), matIndices.end());
        }

        builder.Indices = std::move(groupedIndices);

        std::cout << "[OBJImporter] Loaded " << builder.Materials.size() << " materials, " << builder.SubMeshes.size() << " sub-meshes" << std::endl;

        return true;
    }
}