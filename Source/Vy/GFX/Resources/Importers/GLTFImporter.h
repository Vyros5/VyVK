#pragma once

#include <Vy/GFX/Resources/Importers/ModelImporter.h>

namespace Vy
{
    /**
     * @brief Importer for glTF 2.0 files (.gltf and .glb binary format)
     */
    class GLTFImporter  : public ModelImporter
    {
    public:
        bool load(Model::Builder& builder, const String& filepath, bool flipX, bool flipY, bool flipZ) override;

        TVector<String> getSupportedExtensions() const override { return {"gltf", "glb"}; }

        String getName() const override { return "glTF Importer"; }
    };
}