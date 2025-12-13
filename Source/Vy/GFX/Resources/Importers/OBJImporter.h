#pragma once

#include <Vy/GFX/Resources/Importers/ModelImporter.h>

namespace Vy
{
    /**
     * @brief Importer for Wavefront OBJ files with MTL material support
     */
    class OBJImporter : public ModelImporter
    {
    public:
        bool load(Model::Builder& builder, const String& filepath, bool flipX, bool flipY, bool flipZ) override;

        TVector<String> getSupportedExtensions() const override { return {"obj"}; }

        String getName() const override { return "OBJ Importer"; }
    };
}