#pragma once

#include <VyLib/VyLib.h>

namespace Vy
{
    static constexpr int NO_PARENT  = -1;
    static constexpr int ROOT_JOINT = 0;

    struct SkeletonUBO 
    {
        TVector<Mat4> JointsMatrices;
    };

    struct Joint 
    {
        TStringView Name;
        Mat4        InverseBindMatrix;

        // deformed / animated
        // to be applied to the node matrix a.k.a bind matrix in the world coordinate system,
        // controlled by an animation or a single pose (they come out of gltf animation samplers)
        Vec3 DeformedNodeTranslation{ 0.0f };                    // T
        Quat DeformedNodeRotation   { 1.0f, 0.0f, 0.0f, 0.0f };  // R
        Vec3 DeformedNodeScale      { 1.0f };                    // S

        Mat4 deformedBindMatrix() const 
        {
            // apply scale, rotation, and translation IN THAT ORDER (read from right to the left)
            // to the original undefomed bind matrix
            // dynamically called once per frame
            return 
                glm::scale(Mat4(1.0f), DeformedNodeScale) *           // S
                Mat4(DeformedNodeRotation) *                          // R
                glm::translate(Mat4(1.0f), DeformedNodeTranslation);  // T
        }

        // parents and children for the tree hierachy
        int          ParentJoint;
        TVector<int> Children;
    };

    struct Bone 
    {
        TString       Name;
        I32           ParentIndex;
        TVector<Bone> Children;
        Mat4          OffsetMatrix;
        Mat4          LocalTransform { 1.0f };
        Mat4          GlobalTransform{ 1.0f };
    };

    struct VySkeleton
    {
        void update()
        {
            for (size_t boneIndex = 0; boneIndex < Bones.size(); boneIndex++) 
            {
                Bone& bone = Bones[ boneIndex ];

                // ufbx stores nodes in order where parent nodes always precede child nodes so we can
                // evaluate the transform hierarchy with a flat loop.
                if (bone.ParentIndex >= 0) 
                {
                    bone.GlobalTransform = Bones[ bone.ParentIndex ].GlobalTransform * bone.LocalTransform;
                } 
                else {
                    bone.GlobalTransform = bone.LocalTransform;
                }
                
                SkeletonUbo.JointsMatrices[ boneIndex ] = bone.GlobalTransform * bone.OffsetMatrix;
            }
        }

        bool               IsAnimated = true;
        TStringView        Name;
        TVector<Bone>      Bones;
        TMap<TString, I32> BoneMap;
        TMap<int, int>     GlobalNodeToJointIndex;
        SkeletonUBO        SkeletonUbo;
    };
}