// #include <VyLib/STL/Ref/Ref.h>

// #include <VyLib/STL/Mutex.h>

// namespace Vy
// {
//     static THashSet<void*> s_LiveReferences;
//     static Mutex           s_LiveReferenceMutex;


//     namespace PtrUtil
//     {
//         bool isLiveReference(void* ptr)
//         {
//             std::scoped_lock<Mutex> lock(s_LiveReferenceMutex);

//             return s_LiveReferences.find(ptr) != s_LiveReferences.end();
//         }


//         void addToLiveReferences(void* ptr)
//         {
//             VY_ASSERT(ptr);
            
//             std::scoped_lock<Mutex> lock(s_LiveReferenceMutex);
            
//             s_LiveReferences.insert(ptr);
//         }


//         void removeFromLiveReferences(void* ptr)
//         {
//             VY_ASSERT(ptr);
//             VY_ASSERT(s_LiveReferences.find(ptr) != s_LiveReferences.end());

//             std::scoped_lock<Mutex> lock(s_LiveReferenceMutex);
            
//             s_LiveReferences.erase(ptr);
//         }
//     }
// }