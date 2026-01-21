#pragma once

#include <atomic>
#include <VyLib/Core/Numeric.h>

namespace Vy
{
    template <typename T>
    using TAtomic = std::atomic<T>;

    using TAtomicBool = TAtomic<Bool>;
    using TAtomicI32  = TAtomic<I32>;
    using TAtomicU32  = TAtomic<U32>;
    using TAtomicF32  = TAtomic<F32>;

    using TAtomicU16 = std::atomic_uint16_t;

    namespace StdAtomic
    {
        using std::memory_order_acquire;
        using std::memory_order_release;
        using std::memory_order_relaxed;
        using std::memory_order_acq_rel;
    }


}