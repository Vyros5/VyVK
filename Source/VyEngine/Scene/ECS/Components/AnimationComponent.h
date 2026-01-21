// #pragma once

// #include <VyEngine/GFX/Resources/Model/Model.h>

// namespace Vy
// {
// 	struct AnimationComponent 
//     {
// 		Shared<VyModel> Model;
//         TVector<Mat4>   NodeTransforms; // Global transforms for each node

//         int   CurrentAnimationIndex = -1;
//         float CurrentTime           = 0.0f;
//         float PlaybackSpeed         = 1.0f;
//         bool  IsPlaying             = false;
//         bool  Loop                  = true;

//         AnimationComponent(Shared<VyModel> model = nullptr) : 
//             Model(model)
//         {
//             if (Model)
//             {
//                 NodeTransforms.resize( Model->getNodes().size(), Mat4(1.0f) );
//             }
//         }

//         // Helper methods for UI/Scripts
//         void play(int animationIndex = 0, bool bShouldLoop = true)
//         {
//             if (!Model || animationIndex < 0 || animationIndex >= (int)Model->getAnimations().size()) 
//                 return;

//             CurrentAnimationIndex = animationIndex;
//             CurrentTime           = 0.0f;
//             IsPlaying             = true;
//             Loop                  = bShouldLoop;
//         }

//         void stop()
//         {
//             IsPlaying   = false;
//             CurrentTime = 0.0f;
//         }
// 	};
// }