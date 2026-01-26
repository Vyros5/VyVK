#pragma once

#include <VyEngine/Scene/ECS/Entity.h>

namespace Vy
{
    class VyGameObject final : public VyEntity
    {
    public:
        using UMap = THashMap<EntityHandle, Unique<VyGameObject>>;
        using SMap = THashMap<EntityHandle, Shared<VyGameObject>>;

    public:
        VyGameObject();

        ~VyGameObject() override;

        /**
         * @brief Set name of the game object
         * @param name
         */
        void setName(TStringView name) { m_Name = name; }

        /**
         * @brief Add game object as children
         * @param child game object
         */
        void addChild(VyGameObject& child);

        /**
         * @brief Remove child game object
         * @param child game object
         */
        void removeChild(VyGameObject& child);

        /**
         * @brief Get entity id of the parent
         * @return EntityHandle
         */
        EntityHandle getParentHandle() const { return m_Parent; }

        /**
         * @brief Set parent id for a game object
         * @param parentId
         */
        void setParent(EntityHandle parentId) { m_Parent = parentId; }

        TVector<EntityHandle>& getChildren() { return m_Children; }
        TString                getName()     { return m_Name; }
        const char*            getCName()    { return m_Name.c_str(); }

    private:
        TString               m_Name   = "VyGameObject";
        EntityHandle          m_Parent = kInvalidEntityHandle;
        TVector<EntityHandle> m_Children {};
    };
}