#pragma once

#include <VyEngine/GFX/Resources/Animation/AnimationClip.h>

namespace Vy
{
    class VyAnimations 
    {
        friend class Editor;

    public:
        // used for range-based loops to traverse the array elements in m_AnimationsVector
        struct Iterator 
        {
            // Iterator points to an array element of m_AnimationsVector
            Iterator(Shared<AnimationClip>* pointer) { m_Ptr = pointer; }
            Iterator& operator++();                                                          // pre increment operator (next element)
            bool operator!=(const Iterator& other) { return m_Ptr != other.m_Ptr; }  // unequal operator
            AnimationClip& operator*() { return *(*m_Ptr); }                             // dereference operator

        private:
            Shared<AnimationClip>* m_Ptr;
        };

    public:
        Iterator begin() { return Iterator(&(*m_AnimationsVector.begin())); }
        Iterator end()   { return Iterator(&(*m_AnimationsVector.end())); }
        AnimationClip& operator[](TStringView animation);
        AnimationClip& operator[](U32 index);
        size_t size() const { return m_Animations.size(); }

        void push(const Shared<AnimationClip>& animation);

    public:
        static Unique<VyAnimations> loadAnimationsFromFile(TStringView filePath);
        bool addAnimationFromFile(TStringView filePath);

        void start(TStringView animation);  // by name
        void start(size_t index);           // by index
        void start() { start(0); };         // start animation 0
        void stop();

        void update(VySkeleton& skeleton, U32 frameCounter);

        void setRepeat(bool repeat);
        void setRepeatAll(bool repeat);

        bool        isRunning() const;
        bool        willExpire() const;
        float       dration(TStringView animation) { return m_Animations[animation]->duration(); }
        float       currentFrameTime();
        TStringView name();
        TString     name(U32 index) { return m_AnimationNames[index]; }
        int         index(TStringView animation);

    private:
        TMap<TStringView, Shared<AnimationClip>> m_Animations;
        TVector<Shared<AnimationClip>>           m_AnimationsVector;
        TVector<TString>                         m_AnimationNames;
        AnimationClip*                           m_currentAnimation = nullptr;
        U32                                      m_FrameCounter     = 1;
        TMap<TStringView, int>                   m_NameToIndex;
    };
}