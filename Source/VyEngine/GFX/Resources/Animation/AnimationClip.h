#pragma once 

#include <VyEngine/Core/Timestep.h>
#include <VyEngine/GFX/Resources/Model/Model.h>
#include <VyEngine/GFX/Resources/Animation/Skeleton.h>

namespace Vy
{
    class AnimationClip 
    {
    public:
        enum class Path 
        {
            TRANSLATION,
            ROTATION,
            SCALE
        };

        enum class InterpolationMethod 
        {
            LINEAR,
            STEP,
            CUBICSPLINE
        };

        // struct Channel {
        //	Path path;
        //	int samplerIndex;
        //	int node;
        // };

        // struct Sampler {
        //	TVector<float> timestamps;
        //	TVector<Vec4> valuesToInterpolate;
        //	InterpolationMethod interpolation;
        // };

        struct Frame 
        {
            double Time;
        };

        struct Position : public Frame 
        {
            Vec3 XYZ;
        };

        struct Rotation : public Frame 
        {
            Quat Quaternion;
        };

        struct Scaling : public Frame 
        {
            Vec3 XYZ;
        };

        struct Keyframe 
        {
            float Time;
            Vec3  Position;
            Quat  Rotation;
            Vec3  Scale;
        };

        struct BoneAnimation 
        {
            TVector<Keyframe> Keyframes;
        };

        /// a frame of animation for all of the bones.
        struct AnimNode 
        {
            float         TimeBegin;
            float         Framerate;
            size_t        FrameCount;
            
            TVector<Quat> Rot;
            TVector<Vec3> Pos;
            TVector<Vec3> Scl;
        };

    public:
        // TVector<Sampler> samplers;
        // TVector<Channel> channels;
        TVector<AnimNode>            AnimNodesList;
        TMap<TString, BoneAnimation> BoneAnimations;

    public:
        AnimationClip(TStringView name);

        void start() { m_CurrentKeyFrameTime = m_FirstKeyFrameTime; }
        void stop()  { m_CurrentKeyFrameTime = m_LastKeyFrameTime + 1.0f; }

        void update(VySkeleton& skeleton);

        void setRepeat(bool repeat)                        { m_Repeat = repeat; }
        void setFirstKeyFrameTime(float firstKeyFrameTime) { m_FirstKeyFrameTime = firstKeyFrameTime; }
        void setLastKeyFrameTime(float lastKeyFrameTime)   { m_LastKeyFrameTime = lastKeyFrameTime; }
        void setFramerate(float framerate)                 { m_Framerate = framerate; }
        void setTotalFrameCount(U32 totalFrameCount)       { m_TotalFrameCount = totalFrameCount; }

        bool isRunning()  const { return (m_Repeat || (m_CurrentKeyFrameTime <= m_LastKeyFrameTime)); }
        bool willExpire() const { return (!m_Repeat && ((m_CurrentKeyFrameTime + VyTimestep::count()) > m_LastKeyFrameTime)); }

        TStringView name()             const { return m_Name; }
        float       duration()         const { return m_LastKeyFrameTime - m_FirstKeyFrameTime; }
        float       framerates()       const { return m_Framerate; }
        float       currentFrameTime() const { return m_CurrentKeyFrameTime - m_FirstKeyFrameTime; }
        U32         totalFrameTime()   const { return m_TotalFrameCount; }

    private:
        TStringView m_Name;

        bool m_Repeat = false;

        // relative animation time
        float m_FirstKeyFrameTime   = 0.0f;
        float m_LastKeyFrameTime    = 0.0f;
        float m_Framerate           = 0.0f;
        float m_CurrentKeyFrameTime = 0.0f;
        U32   m_TotalFrameCount       = 0;
    };
}