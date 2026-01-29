// #include <VyEngine/GFX/Resources/Animation/AnimationClip.h>

// namespace Vy
// {
//     AnimationClip::AnimationClip(TStringView name)
//         : m_Name(name) 
//     {
//     }

//     void AnimationClip::update(VySkeleton& skeleton) 
//     {
//         if (!isRunning()) 
//         {
//             return;
//         }

//         m_CurrentKeyFrameTime += VyTimestep::count();

//         if (m_Repeat && (m_CurrentKeyFrameTime > m_LastKeyFrameTime)) {
//             m_CurrentKeyFrameTime = m_FirstKeyFrameTime;
//         }

//         float frameTime = (m_CurrentKeyFrameTime - m_FirstKeyFrameTime) * 30;
//         U32   f0        = glm::min((U32)frameTime + 0, m_TotalFrameCount - 1);
//         U32   f1        = glm::min((U32)frameTime + 1, m_TotalFrameCount - 1);
//         float t         = glm::min(frameTime - (float)f0, 1.0f);

//         for (U32 i = 0; i < skeleton.Bones.size(); ++i) 
//         {
//             Bone&    bone      = skeleton.Bones[ i ];
//             AnimNode animNodes = AnimNodesList [ i ];

//             Quat rot   = glm::lerp( animNodes.Rot[ f0 ], animNodes.Rot[ f1 ], t );
//             Vec3 pos   = glm::mix ( animNodes.Pos[ f0 ], animNodes.Pos[ f1 ], t );
//             Vec3 scale = glm::mix ( animNodes.Scl[ f0 ], animNodes.Scl[ f1 ], t );

//             //bone.localTransform = glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot) * glm::scale(glm::mat4(1.0f), scale);
//             bone.LocalTransform = glm::scale(Mat4(1.0f), scale) * glm::mat4_cast(rot) * glm::translate(Mat4(1.0f), pos);
//         }
//     }
// }