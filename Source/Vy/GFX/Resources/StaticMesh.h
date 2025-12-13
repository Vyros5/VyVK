#pragma once

#include <Vy/GFX/Backend/Device.h>
#include <Vy/GFX/Backend/Resources/Buffer.h>

#include <VyLib/Util/Hash.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// Forward declarations for Assimp
struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;

namespace Vy
{
    /**
     * @struct VyVertex
     *
     * @brief Represents a single vertex with position, color, normal, tangent, and texture coordinates.
     */
    struct alignas(16) VyVertex 
    {
        Vec3 Position {}; // Position of the vertex. (1 unused)
        Vec3 Normal   {}; // Normal vector for lighting calculations. (1 unused)
        Vec3 Color    {}; // Color vector. (1 unused)
        Vec2 UV       {}; // Texture coordinates. (2 unused)

        bool operator==(const VyVertex& other) const 
        {
            return 
                Position == other.Position && 
                Normal   == other.Normal   && 
                Color    == other.Color    && 
                UV       == other.UV;
        }
    };


    class VyStaticMesh
    {
    public:
        /**
         * @brief Retrieves the binding descriptions for vertex input.
         *
         * @return A vector of VkVertexInputBindingDescription.
         */
        static TVector<VkVertexInputBindingDescription> vertexBindingDescriptions();

        /**
         * @brief Retrieves the attribute descriptions for vertex input.
         *
         * @return A vector of VkVertexInputAttributeDescription.
         */
        static TVector<VkVertexInputAttributeDescription> vertexAttributeDescriptions();

        /**
         * @brief Retrieves the attribute description for vertex input position.
         *
         * @return A vector of VkVertexInputAttributeDescription (only position).
         */
        static TVector<VkVertexInputAttributeDescription> vertexAttributeDescriptionOnlyPositon();

        struct Builder 
        {
            TVector<VyVertex> Vertices{};
            TVector<U32>      Indices{};

            void loadModel(const Path& file);

        private:
            void processNode(aiNode* node, const aiScene* scene);
            void processMesh(aiMesh* mesh, const aiScene* scene);

            void loadMaterialTextures(aiMaterial* mat, int type, const String& directory);
        };

        static Unique<VyStaticMesh> create(const Path& file);

        static Unique<VyStaticMesh> create(TVector<VyVertex>& vertices, TVector<U32>& indices);

    public:
        VyStaticMesh(TVector<VyVertex>& vertices, TVector<U32>& indices);

        ~VyStaticMesh(); // override;

        VyStaticMesh(const VyStaticMesh&) = delete;
        VyStaticMesh& operator=(const VyStaticMesh&) = delete;

        /**
         * @brief Binds the model's vertex and index buffers to a command buffer.
         * 
         * @param cmdBuffer The Vulkan command buffer.
         */
        void bind(VkCommandBuffer cmdBuffer);
        
        /**
         * @brief Draws the model using the bound buffers.
         * 
         * @param cmdBuffer The Vulkan command buffer.
         */
        void draw(VkCommandBuffer cmdBuffer);

    private:
        /**
         * @brief Creates and allocates vertex buffers.
         */
        void createVertexBuffers(TVector<VyVertex>& vertices);

        /**
         * @brief Creates and allocates index buffers.
         */
        void createIndexBuffers(TVector<U32>& indices);


        Unique<VyBuffer> m_VertexBuffer;
        U32              m_VertexCount;

        Unique<VyBuffer> m_IndexBuffer;
        U32              m_IndexCount;
    };
}


template <>
struct std::hash<Vy::VyVertex> 
{
    size_t operator()(Vy::VyVertex const& vertex) const noexcept 
    {
        size_t seed = 0;
        
        Vy::Hash::hashCombine(seed, 
            vertex.Position, 
            vertex.Normal, 
            vertex.Color, 
            vertex.UV
        );
        
        return seed;
    }
};

// namespace Vy
// {
//     /**
//      * @struct VyVertex
//      *
//      * @brief Represents a single vertex with position, color, normal, tangent, and texture coordinates.
//      */
//     struct alignas(16) VyVertex 
//     {
//         Vec3 Position {}; // Position of the vertex. (1 unused)
//         Vec3 Normal   {}; // Normal vector for lighting calculations. (1 unused)
//         // Vec4 Tangent  {}; // Tangent vector for lighting calculations.
//         Vec3 Color    {}; // Color vector. (1 unused)
//         Vec2 UV       {}; // Texture coordinates. (2 unused)

//         bool operator==(const VyVertex& other) const 
//         {
//             return 
//                 Position == other.Position && 
//                 Normal   == other.Normal   && 
//                 // Tangent  == other.Tangent  && 
//                 Color    == other.Color    && 
//                 UV       == other.UV;
//         }
//     };


//     class VyStaticMesh
//     {
//     public:
//         /**
//          * @brief Retrieves the binding descriptions for vertex input.
//          *
//          * @return A vector of VkVertexInputBindingDescription.
//          */
//         static TVector<VkVertexInputBindingDescription> vertexBindingDescriptions();

//         /**
//          * @brief Retrieves the attribute descriptions for vertex input.
//          *
//          * @return A vector of VkVertexInputAttributeDescription.
//          */
//         static TVector<VkVertexInputAttributeDescription> vertexAttributeDescriptions();

//         /**
//          * @brief Retrieves the attribute description for vertex input position.
//          *
//          * @return A vector of VkVertexInputAttributeDescription (only position).
//          */
//         static TVector<VkVertexInputAttributeDescription> vertexAttributeDescriptionOnlyPositon();

//         // struct Builder 
//         // {
//         //     TVector<VyVertex> Vertices{};
//         //     TVector<U32>      Indices{};

//         //     void loadModel(const Path& file);
//         // };

//         static Unique<VyStaticMesh> create(const Path& file);
//         static Unique<VyStaticMesh> create(TVector<VyVertex>& vertices, TVector<U32>& indices);

//         VyStaticMesh(TVector<VyVertex>& vertices, TVector<U32>& indices);

//         ~VyStaticMesh(); // override;

//         VyStaticMesh(const VyStaticMesh&) = delete;
//         VyStaticMesh& operator=(const VyStaticMesh&) = delete;

//         /**
//          * @brief Binds the model's vertex and index buffers to a command buffer.
//          * 
//          * @param cmdBuffer The Vulkan command buffer.
//          */
//         void bind(VkCommandBuffer cmdBuffer);
        
//         /**
//          * @brief Draws the model using the bound buffers.
//          * 
//          * @param cmdBuffer The Vulkan command buffer.
//          */
//         void draw(VkCommandBuffer cmdBuffer);

//     private:
//         /**
//          * @brief Creates and allocates vertex buffers.
//          */
//         void createVertexBuffers(TVector<VyVertex>& vertices);

//         /**
//          * @brief Creates and allocates index buffers.
//          */
//         void createIndexBuffers(TVector<U32>& indices);


//         Unique<VyBuffer> m_VertexBuffer;
//         U32              m_VertexCount;

//         Unique<VyBuffer> m_IndexBuffer;
//         U32              m_IndexCount;
//     };
// }

// template <>
// struct std::hash<Vy::VyVertex> 
// {
//     size_t operator()(Vy::VyVertex const& vertex) const noexcept 
//     {
//         size_t seed = 0;
        
//         Vy::Hash::hashCombine(seed, 
//             vertex.Position, 
//             vertex.Normal, 
//             // vertex.Tangent, 
//             vertex.Color, 
//             vertex.UV
//         );
        
//         return seed;
//     }
// };