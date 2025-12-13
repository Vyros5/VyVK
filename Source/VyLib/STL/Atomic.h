#pragma once

#include <atomic>
#include <VyLib/Common/Numeric.h>

namespace Vy
{
    template <typename T>
    using Atomic = std::atomic<T>;

    using AtomicBool = Atomic<Bool>;
    using AtomicI32  = Atomic<I32>;
    using AtomicU32  = Atomic<U32>;
    using AtomicF32  = Atomic<F32>;

    using AtomicU16 = std::atomic_uint16_t;

    namespace StdAtomic
    {
        using std::memory_order_acquire;
        using std::memory_order_release;
        using std::memory_order_relaxed;
        using std::memory_order_acq_rel;
    }


}