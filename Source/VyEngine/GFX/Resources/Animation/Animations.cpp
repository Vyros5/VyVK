#include <VyEngine/GFX/Resources/Animation/Animations.h>

#include <VyEngine/GFX/Resources/Animation/Skeleton.h>

#include <VyEngine/GFX/Resources/Model/UFBXImporter.h>

namespace Vy
{
    Unique<VyAnimations> VyAnimations::loadAnimationsFromFile(TStringView filePath) 
    {
        UFBXImporter loader{filePath.data()};
        
        if (!loader.loadAnimations()) 
        {
            VY_ERROR_TAG("VyAnimations", "Failed to load Model file {0}", filePath.data());
            return nullptr;
        }

        return std::move( loader.Animations );
    }

    bool VyAnimations::addAnimationFromFile(TStringView filePath) 
    {
        UFBXImporter loader{filePath.data()};
        return loader.loadAnimations();
        // VY_ERROR_TAG("VyAnimations", "Failed to load Model file {0}", filePath.data());
        //	return nullptr;
        //}

        // return std::move(loader.animations);
    }

    AnimationClip& VyAnimations::operator[](TStringView animation) 
    {
        return *m_Animations[ animation ];
    }

    AnimationClip& VyAnimations::operator[](U32 index) 
    {
        return *m_AnimationsVector[ index ];
    }

    void VyAnimations::push(const Shared<AnimationClip>& animation) 
    {
        if (animation) 
        {
            m_Animations[ animation->name() ] = animation;
            
            m_AnimationsVector.push_back( animation );
            m_AnimationNames  .push_back( animation->name().data() );

            m_NameToIndex[ animation->name() ] = static_cast<int>( m_AnimationsVector.size() - 1 );
        } 
        else {
            VY_ERROR_TAG("VyAnimations", "VyAnimations::push: Animation is empty!");
        }
    }

    void VyAnimations::start(TStringView animation) 
    {
        AnimationClip* pCurrentAnimation = m_Animations[ animation ].get();

        if (m_currentAnimation) 
        {
            m_currentAnimation = pCurrentAnimation;

            m_currentAnimation->start();
        }
    }

    void VyAnimations::start(size_t index) 
    {
        if (!(index < m_AnimationsVector.size())) 
        {
            VY_ERROR_TAG("VyAnimations", "VyAnimations::start(U32 index) out of bounds!");
            return;
        }

        AnimationClip* pCurrentAnimation = m_AnimationsVector[ index ].get();
        
        if (pCurrentAnimation) 
        {
            m_currentAnimation = pCurrentAnimation;

            m_currentAnimation->start();
        }
    }


    void VyAnimations::stop() 
    {
        if (m_currentAnimation) 
        {
            m_currentAnimation->stop();
        }
    }

    void VyAnimations::update(VySkeleton& skeleton, U32 frameCounter) 
    {
        if (m_FrameCounter != frameCounter) 
        {
            m_FrameCounter = frameCounter;

            if (m_currentAnimation) 
            {
                m_currentAnimation->update( skeleton );
            }
        }
    }

    void VyAnimations::setRepeat(bool repeat) 
    {
        if (m_currentAnimation) 
        {
            m_currentAnimation->setRepeat(repeat);
        }
    }

    void VyAnimations::setRepeatAll(bool repeat) 
    {
        for (auto& animation : m_AnimationsVector) 
        {
            animation->setRepeat(repeat);
        }
    }

    bool VyAnimations::isRunning() const 
    {
        if (m_currentAnimation) 
        {
            return m_currentAnimation->isRunning();
        } 
        else {
            return false;
        }
    }

    bool VyAnimations::willExpire() const 
    {
        if (m_currentAnimation) 
        {
            return m_currentAnimation->willExpire();
        } 
        else {
            return false;
        }
    }

    float VyAnimations::currentFrameTime() 
    {
        if (m_currentAnimation) 
        {
            return m_currentAnimation->currentFrameTime();
        } 
        else {
            return 0.0f;
        }
    }

    TStringView VyAnimations::name() 
    {
        if (m_currentAnimation) 
        {
            return m_currentAnimation->name();
        } 
        else {
            return "";
        }
    }

    
    int VyAnimations::index(TStringView animation) 
    {
        bool bFound = false;

        for (auto& element : m_AnimationsVector) 
        {
            if (element->name() == animation) 
            {
                bFound = true;
                break;
            }
        }

        if (bFound) 
        {
            return m_NameToIndex[ animation ];
        } 
        else {
            return -1;
        }
    }


    VyAnimations::Iterator& VyAnimations::Iterator::operator++() 
    {
        ++m_Ptr;

        return *this;
    }
}