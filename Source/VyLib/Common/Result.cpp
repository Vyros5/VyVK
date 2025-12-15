// #include <VyLib/Common/Result.h>

// namespace Vy
// {
// namespace Memory
// {
//     bool isAligned(const void *pPtr, const size_t alignment)
//     {
//         const UPtr addr = reinterpret_cast<UPtr>(pPtr);
//         return (addr & (alignment - 1)) == 0;
//     }

//     bool isAligned(const size_t address, const size_t alignment)
//     {
//         return (address & (alignment - 1)) == 0;
//     }

//     void* allocate(const size_t size)
//     {
//         void *ptr = std::malloc(size);
//         // TKIT_PROFILE_MARK_ALLOCATION(ptr, size);
//         return ptr;
//     }

//     void deallocate(void *pPtr)
//     {
//         // TKIT_PROFILE_MARK_DEALLOCATION(pPtr);
//         std::free(pPtr);
//     }

//     void* allocateAligned(const size_t size, size_t alignment)
//     {
//         void *ptr = nullptr;
//         alignment = (alignment + sizeof(void *) - 1) & ~(sizeof(void *) - 1);
//     #ifdef VY_PLATFORM_WINDOWS
//         ptr = _aligned_malloc(size, alignment);
//     #else
//         int result = posix_memalign(&ptr, alignment, size);
//         TKIT_UNUSED(result); // Sould do something with this at some point
//     #endif
//         // TKIT_PROFILE_MARK_ALLOCATION(ptr, size);
//         return ptr;
//     }

//     void deallocateAligned(void *pPtr)
//     {
//         // TKIT_PROFILE_MARK_DEALLOCATION(pPtr);
//     #ifdef VY_PLATFORM_WINDOWS
//         _aligned_free(pPtr);
//     #else
//         std::free(pPtr);
//     #endif
//     }

//     void* forwardCopy(void *pDst, const void *pSrc, size_t size)
//     {
//         return std::memcpy(pDst, pSrc, size);
//     }

//     void* backwardCopy(void *pDst, const void *pSrc, size_t size)
//     {
//         return std::memmove(pDst, pSrc, size);
//     }
// }
// }