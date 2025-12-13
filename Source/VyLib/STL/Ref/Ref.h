// #pragma once

// #include <VyLib/Core/Assert.h>

// #include <VyLib/STL/Atomic.h>
// #include <VyLib/STL/Containers.h>
// #include <VyLib/STL/Utility.h>
// #include <VyLib/STL/Mutex.h>

// #include <memory>
// #include <type_traits>

// namespace Vy
// {
// 	/**
// 	 * @brief Base class for objects that can be reference-counted.
// 	 *
// 	 * Provides thread-safe reference counting capabilities that can be used by smart pointers.
//      * 
// 	 * Objects inheriting from this class can be managed by Ref<T>.
// 	 */
// 	class RefCounted
//     {
// 	public:
// 		/**
// 		 * @brief Default constructor initializes reference count to 0.
// 		 */
// 		RefCounted() = default;

// 		/**
// 		 * @brief Copy constructor maintains the reference count at 0.
// 		 *
// 		 * When an object is copied, the new instance starts with a fresh reference count.
// 		 */
// 		RefCounted(const RefCounted&) noexcept 
//         {
//         }

// 		/**
// 		 * @brief Copy assignment operator doesn't affect reference count.
// 		 *
// 		 * Reference count is associated with object identity, not with its contents.
// 		 */
// 		RefCounted& operator=(const RefCounted&) noexcept 
//         { 
//             return *this; 
//         }

// 		/**
// 		 * @brief Move constructor maintains the reference count at 0.
// 		 */
// 		RefCounted(RefCounted&&) noexcept 
//         {
//         }

// 		/**
// 		 * @brief Move assignment operator doesn't affect reference count.
// 		 */
// 		RefCounted& operator=(RefCounted&&) noexcept 
//         { 
//             return *this; 
//         }

// 		/**
// 		 * @brief Virtual destructor for proper polymorphic behavior.
// 		 */
// 		virtual ~RefCounted() = default;

// 		/**
// 		 * @brief Increments the reference count.
// 		 * @return The new reference count.
// 		 */
// 		U32 incRefCount() const noexcept 
//         { 
//             return ++m_RefCount; 
//         }

// 		/**
// 		 * @brief Decrements the reference count.
// 		 * @return The new reference count.
// 		 */
// 		U32 decRefCount() const noexcept
// 		{
// 			VY_ASSERT(m_RefCount > 0, "Reference count is already 0");

// 			return --m_RefCount;
// 		}

// 		/**
// 		 * @brief Gets the current reference count.
//          * 
// 		 * @return The current reference count.
// 		 */
// 		U32 getRefCount() const noexcept 
//         { 
//             return m_RefCount; 
//         }

// 	private:
// 		/// Using mutable to allow const objects to be reference counted
// 		mutable Atomic<U32> m_RefCount{0};
// 	};

//     // ============================================================================================

// #pragma region ControlBlock 

//     namespace Internal
//     {
// 		/**
// 		 * @brief Control block for managing weak references to an object.
// 		 *
// 		 * The ControlBlock is responsible for tracking weak references to an object
// 		 * even after the object itself has been destroyed. 
//          * 
//          * It maintains:
// 		 *  
//          *  1. A pointer to the actual object (which becomes nullptr when the object is destroyed)
// 		 *  
//          *  2. A count of weak references pointing to this control block
// 		 *
// 		 * When an object is destroyed but weak references to it still exist, the control
// 		 * block remains alive (with m_Ptr set to nullptr) until all weak references are gone.
// 		 *
// 		 * @tparam T The type of object being referenced.
// 		 */
// 		template <typename T>
// 		class ControlBlock
// 		{
// 		public:
// 		    /**
// 		     * @brief Constructs a control block for the specified object
// 		     *
// 		     * @param ptr Pointer to the object being tracked
// 		     */
// 		    explicit ControlBlock(T* ptr) noexcept : 
//                 m_Ptr(ptr), 
//                 m_WeakCount(0) 
//             {
//             }

// 		    /**
// 		     * @brief Increments the weak reference count
// 		     *
// 		     * Called when a new WeakRef is created or copied to point to this object
// 		     */
// 		    void incWeakCount() noexcept 
//             { 
//                 ++m_WeakCount; 
//             }

// 		    /**
// 		     * @brief Decrements the weak reference count
// 		     *
// 		     * When the weak count reaches zero and the object pointer is nullptr
// 		     * (indicating the object has been destroyed), the control block
// 		     * deletes itself as it's no longer needed.
// 		     */
// 		    void decWeakCount() noexcept
// 		    {
// 		        if (--m_WeakCount == 0 && m_Ptr == nullptr)
// 		        {
// 		            delete this;
// 		        }
// 		    }

// 		    /**
// 		     * @brief Gets the pointer to the managed object
// 		     *
// 		     * @return The pointer to the object, or nullptr if the object has been destroyed
// 		     */
// 		    T* getPtr() const noexcept 
//             { 
//                 return m_Ptr; 
//             }

// 		    /**
// 		     * @brief Sets the object pointer
// 		     *
// 		     * This is typically called with nullptr when the object is being destroyed
// 		     * to indicate that the object is no longer valid.
// 		     *
// 		     * @param ptr The new object pointer value
// 		     */
// 		    void setPtr(T* ptr) noexcept
// 		    {
// 		        m_Ptr = ptr;
// 		    }

// 		    /**
// 		     * @brief Gets the current weak reference count
// 		     *
// 		     * @return The number of weak references pointing to this control block
// 		     */
// 		    U32 getWeakCount() const noexcept
// 		    {
// 		        return m_WeakCount;
// 		    }

// 		private:
//             // Pointer to the managed object, or nullptr if destroyed.
// 		    T* m_Ptr;

//             // Number of weak references to this object.
// 		    Atomic<U32> m_WeakCount;
// 		};

//         // ========================================================================================

// 		/**
// 		 * @brief Registry for managing control blocks for weak references.
// 		 *
// 		 * This class maintains a mapping from object pointers to their associated control blocks,
// 		 * allowing weak references to locate the control block for an object they're referencing.
// 		 * It ensures that multiple weak references to the same object share the same control block.
// 		 *
// 		 * The registry is implemented as a singleton to provide global access while ensuring
// 		 * there's only one instance managing all control blocks for a given type T.
// 		 *
// 		 * @tparam T The type of objects being tracked in this registry.
// 		 */
// 		template <typename T>
// 		class ControlBlockRegistry
// 		{
// 		public:
// 		    /**
// 		     * @brief get the singleton instance of the registry
// 		     *
// 		     * @return A reference to the singleton instance
// 		     */
// 		    static ControlBlockRegistry& getInstance()
// 		    {
// 		        static ControlBlockRegistry instance;

// 		        return instance;
// 		    }

// 		    /**
// 		     * @brief get or create a control block for the specified object pointer
// 		     *
// 		     * If a control block already exists for the given pointer, it returns that block.
// 		     * Otherwise, it creates a new control block, registers it, and returns it.
// 		     *
// 		     * @param ptr Pointer to the object for which to get/create a control block
// 		     * @return Pointer to the control block, or nullptr if ptr is nullptr
// 		     */
// 		    ControlBlock<T>* getControlBlock(T* ptr)
// 		    {
// 		        if (!ptr)
// 		            return nullptr;

// 		        LockGuard<Mutex> lock(m_Mutex);

// 		        auto it = m_Blocks.find(ptr);

// 		        if (it != m_Blocks.end())
// 		        {
// 		            return it->second;
// 		        }

// 		        auto block = new Internal::ControlBlock<T>(ptr);

// 		        m_Blocks[ptr] = block;

// 		        return block;
// 		    }

// 		    /**
// 		     * @brief Remove the control block associated with the specified object pointer
// 		     *
// 		     * This method is called when an object is being destroyed. It sets the object pointer
// 		     * in the control block to nullptr to indicate that the object is no longer valid.
// 		     * If there are no weak references to the object, the control block itself is deleted.
// 		     *
// 		     * @param ptr Pointer to the object whose control block should be removed
// 		     */
// 		    void removeControlBlock(T* ptr)
// 		    {
// 		        if (!ptr)
// 		            return;

// 		        LockGuard<Mutex> lock(m_Mutex);

// 		        auto it = m_Blocks.find(ptr);
// 		        if (it != m_Blocks.end())
// 		        {
// 		            it->second->setPtr(nullptr);

// 		            if (it->second->getWeakCount() == 0)
// 		            {
// 		                delete it->second;
// 		            }

// 		            m_Blocks.erase(it);
// 		        }
// 		    }

// 		private:
// 		    /**
// 		     * @brief Private constructor to enforce singleton pattern.
// 		     */
// 		    ControlBlockRegistry() = default;

//             /**
//              * @brief Destructor cleans up all remaining control blocks.
//              */
// 		    ~ControlBlockRegistry()
// 		    {
// 		        for (auto& pair : m_Blocks)
// 		        {
// 		            delete pair.second;
// 		        }
// 		    }

// 		    THashMap<T*, Internal::ControlBlock<T>*> m_Blocks;
// 		    Mutex                                    m_Mutex;
// 		};
//     } // namespace Internal

// #pragma endregion ControlBlock

//     // ============================================================================================

// 	template <typename T>
// 	class WeakRef;

//     // ============================================================================================

// #pragma region Ref

// 	/**
// 	 * @brief A reference-counting smart pointer that manages shared ownership of objects.
// 	 *
// 	 * The Ref class provides a reference-counting ownership mechanism where multiple
// 	 * Ref instances can share ownership of a single object. The object is destroyed
// 	 * when the last Ref pointing to it is destroyed or reset.
// 	 *
// 	 * @tparam T The type of the managed object.
// 	 */
// 	template <typename T>
// 	class Ref
// 	{
// 	public:

// 		/**
// 		 * @brief Default constructor creates a null reference.
// 		 */
// 		constexpr Ref() noexcept = default;


// 		/**
// 		 * @brief Constructor from nullptr creates a null reference.
// 		 */
// 		constexpr Ref(std::nullptr_t) noexcept {}


// 		/**
// 		 * @brief Constructor from raw pointer. Takes ownership of the object.
// 		 *
// 		 * This constructor increments the reference count of the object.
// 		 *
// 		 * @param ptr Pointer to the object to manage.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		explicit Ref(U* ptr) noexcept : 
//             m_Ptr(ptr)
// 		{
// 			internalAddRef();
// 		}


// 		/**
// 		 * @brief Copy constructor. Shares ownership of the object.
// 		 *
// 		 * This constructor increments the reference count of the object.
// 		 *
// 		 * @param other The Ref to copy from.
// 		 */
// 		Ref(const Ref& other) noexcept : 
//             m_Ptr(other.m_Ptr)
// 		{
// 			internalAddRef();
// 		}


// 		/**
// 		 * @brief Copy constructor with type conversion. Shares ownership of the object.
// 		 *
// 		 * This constructor increments the reference count of the object and allows
// 		 * converting between compatible types.
// 		 *
// 		 * @tparam U The type of the other Ref.
// 		 * @param other The Ref to copy from.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		Ref(const Ref<U>& other) noexcept : 
//             m_Ptr(other.get())
// 		{
// 			internalAddRef();
// 		}


// 		/**
// 		 * @brief Move constructor. Takes ownership from another Ref.
// 		 *
// 		 * This constructor doesn't change the reference count of the object.
// 		 *
// 		 * @param other The Ref to move from.
// 		 */
// 		Ref(Ref&& other) noexcept : 
//             m_Ptr(other.m_Ptr) 
//         {
//             other.m_Ptr = nullptr; 
//         }


// 		/**
// 		 * @brief Move constructor with type conversion. Takes ownership from another Ref.
// 		 *
// 		 * This constructor doesn't change the reference count of the object and allows
// 		 * converting between compatible types.
// 		 *
// 		 * @tparam U The type of the other Ref.
// 		 * @param other The Ref to move from.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		Ref(Ref<U>&& other) noexcept : 
//             m_Ptr(other.get()) 
//         {
//             other.m_Ptr = nullptr; 
//         }


// 		/**
// 		 * @brief Constructor from std::shared_ptr. Shares ownership of the object.
// 		 *
// 		 * This constructor allows interoperability with std::shared_ptr.
// 		 *
// 		 * @param shared The std::shared_ptr to convert from.
// 		 */
// 		explicit Ref(const std::shared_ptr<T>& shared) noexcept : 
//             m_Ptr(shared.get())
// 		{
// 			internalAddRef();
// 		}


// 		/**
// 		 * @brief Constructor from WeakRef. Obtains a strong reference if available.
// 		 *
// 		 * This constructor attempts to obtain a strong reference from a WeakRef.
// 		 * If the WeakRef has expired, the Ref will be null.
// 		 *
// 		 * @param weak The WeakRef to convert from.
// 		 */
// 		explicit Ref(const WeakRef<T>& weak) noexcept;


// 		/**
// 		 * @brief Destructor. Decrements the reference count of the object.
// 		 *
// 		 * If the reference count reaches 0, the object is destroyed.
// 		 */
// 		~Ref()
// 		{
// 			internalRelease();
// 		}


// 		/**
// 		 * @brief Copy assignment operator. Shares ownership of the object.
// 		 *
// 		 * This operator increments the reference count of the assigned object
// 		 * and decrements the reference count of the previously managed object.
// 		 *
// 		 * @param other The Ref to copy from.
// 		 * @return Reference to this Ref.
// 		 */
// 		Ref& operator=(const Ref& other) noexcept
// 		{
// 			if (this != &other)
// 			{
// 				internalRelease();

// 				m_Ptr = other.m_Ptr;
				
//                 internalAddRef();
// 			}

// 			return *this;
// 		}


// 		/**
// 		 * @brief Copy assignment operator with type conversion. Shares ownership of the object.
// 		 *
// 		 * This operator increments the reference count of the assigned object,
// 		 * decrements the reference count of the previously managed object, and allows
// 		 * converting between compatible types.
// 		 *
// 		 * @tparam U The type of the other Ref.
// 		 * @param other The Ref to copy from.
// 		 * @return Reference to this Ref.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		Ref& operator=(const Ref<U>& other) noexcept
// 		{
// 		    /// Self-assignment or assigning same pointer: do nothing
// 		    if (static_cast<const void*>(this) == static_cast<const void*>(&other) || m_Ptr == other.get())
//             {
//                 return *this;
//             }

//             internalRelease();

//             m_Ptr = other.get();

//             internalAddRef();

//             return *this;
// 		}


// 		/**
// 		 * @brief Move assignment operator. Takes ownership from another Ref.
// 		 *
// 		 * This operator decrements the reference count of the previously managed object
// 		 * and takes ownership of the object from the other Ref without changing its
// 		 * reference count.
// 		 *
// 		 * @param other The Ref to move from.
// 		 * @return Reference to this Ref.
// 		 */
// 		Ref& operator=(Ref&& other) noexcept
// 		{
// 			if (this != &other)
// 			{
// 				internalRelease();

// 				m_Ptr       = other.m_Ptr;
// 				other.m_Ptr = nullptr;
// 			}

// 			return *this;
// 		}


// 		/**
// 		 * @brief Move assignment operator with type conversion. Takes ownership from another Ref.
// 		 *
// 		 * This operator decrements the reference count of the previously managed object,
// 		 * takes ownership of the object from the other Ref without changing its reference count,
// 		 * and allows converting between compatible types.
// 		 *
// 		 * @tparam U The type of the other Ref.
// 		 * @param other The Ref to move from.
// 		 * @return Reference to this Ref.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		Ref& operator=(Ref<U>&& other) noexcept
// 		{
// 		    // Self-move or moving same pointer: do nothing
// 		    if (static_cast<void*>(this) == static_cast<void*>(&other) || m_Ptr == other.get())
//             {
//                 return *this;
//             }

//             internalRelease();

//             m_Ptr       = other.get();
//             other.m_Ptr = nullptr;

//             return *this;
// 		}


// 		/**
// 		 * @brief Assignment operator from nullptr. Resets the Ref.
// 		 *
// 		 * This operator decrements the reference count of the previously managed object
// 		 * and sets the Ref to null.
// 		 *
// 		 * @return Reference to this Ref.
// 		 */
// 		Ref& operator=(std::nullptr_t) noexcept
// 		{
// 			internalRelease();

// 			return *this;
// 		}


// 		/**
// 		 * @brief Dereference operator. Provides access to the managed object.
// 		 *
// 		 * @return Reference to the managed object.
// 		 */
// 		T& operator*() const noexcept
// 		{
// 			VY_ASSERT(m_Ptr, "Dereferencing null Ref");

// 			return *m_Ptr;
// 		}


// 		/**
// 		 * @brief Arrow operator. Provides access to the managed object's members.
// 		 *
// 		 * @return Pointer to the managed object.
// 		 */
// 		T* operator->() const noexcept
// 		{
// 			VY_ASSERT(m_Ptr, "Accessing member of null Ref");

// 			return m_Ptr;
// 		}


// 		/**
// 		 * @brief Boolean conversion operator. Checks if the Ref is not null.
// 		 *
// 		 * @return True if the Ref is not null, false otherwise.
// 		 */
// 		explicit operator bool() const noexcept 
//         { 
//             return m_Ptr != nullptr; 
//         }


// 		/**
// 		 * @brief Gets the raw pointer to the managed object.
// 		 *
// 		 * @return Pointer to the managed object.
// 		 */
// 		T* get() const noexcept 
//         { 
//             return m_Ptr; 
//         }


// 		/**
// 		 * @brief Resets the Ref to null or to manage a new object.
// 		 *
// 		 * This method decrements the reference count of the previously managed object
// 		 * and sets the Ref to manage a new object or to null if no object is provided.
// 		 *
// 		 * @param ptr Pointer to the new object to manage, or nullptr.
// 		 */
// 		void reset(T* ptr = nullptr) noexcept
// 		{
// 			internalRelease();

// 			m_Ptr = ptr;
			
//             internalAddRef();
// 		}


// 		/**
// 		 * @brief Checks if this Ref is the only one managing the object.
// 		 *
// 		 * @return True if the reference count is 1, false otherwise or if null.
// 		 */
// 		bool isUnique() const noexcept 
//         { 
//             return m_Ptr && m_Ptr->getRefCount() == 1; 
//         }


// 		/**
// 		 * @brief Converts this Ref to a Ref of another type using static_cast.
// 		 *
// 		 * @tparam U The type to convert to.
// 		 * @return A Ref<U> managing the same object.
// 		 */
// 		template <typename U>
// 		Ref<U> As() const noexcept 
//         { 
//             return Ref<U>(static_cast<U*>(m_Ptr)); 
//         }


// 		/**
// 		 * @brief Converts this Ref to a Ref of another type using dynamic_cast.
// 		 *
// 		 * @tparam U The type to convert to.
// 		 * @return A Ref<U> managing the same object, or null if the cast fails.
// 		 */
// 		template <typename U>
// 		Ref<U> DynamicCast() const noexcept
// 		{
// 			if (U* cast = dynamic_cast<U*>(m_Ptr))
//             {
//                 return Ref<U>(cast);
//             }

//             return Ref<U>();
// 		}


// 		/**
// 		 * @brief Converts this Ref to a std::shared_ptr.
// 		 *
// 		 * This method creates a std::shared_ptr from this Ref with a custom deleter
// 		 * that decrements the reference count. This allows interoperability with
// 		 * functions that expect std::shared_ptr.
// 		 *
// 		 * @return A std::shared_ptr managing the same object.
// 		 */
// 		std::shared_ptr<T> toSharedPtr() const noexcept
// 		{
// 			if (!m_Ptr)
//             {
//                 return nullptr;
//             }

// 			/// Increment the ref count for the shared_ptr
// 			internalAddRef();

// 			/// Create a shared_ptr with a custom deleter that decrements the ref count
// 			return std::shared_ptr<T>(m_Ptr, [](T* ptr)
// 			{
// 				if (ptr && ptr->decRefCount() == 0)
//                 {
//                     delete ptr;
//                 }
// 			});
// 		}


// 		/**
// 		 * @brief Swaps the contents of this Ref with another.
// 		 *
// 		 * @param other The Ref to swap with.
// 		 */
// 		void swap(Ref& other) noexcept
// 		{
// 			std::swap(m_Ptr, other.m_Ptr);
// 		}


// 		/**
// 		 * @brief Checks if the Ref is not null.
// 		 *
// 		 * @return True if the Ref is not null, false otherwise.
// 		 */
// 		bool isValid() const noexcept 
//         { 
//             return m_Ptr != nullptr; 
//         }


// 		/**
// 		 * @brief Gets the reference count of the managed object.
// 		 *
// 		 * @return The reference count, or 0 if the Ref is null.
// 		 */
// 		U32 useCount() const noexcept
//         { 
//             return m_Ptr ? m_Ptr->getRefCount() : 0; 
//         }


// 		/**
// 		 * @brief Equality operator. Compares the managed objects.
// 		 *
// 		 * @param other The Ref to compare with.
// 		 * @return True if both Refs manage the same object, false otherwise.
// 		 */
// 		bool operator==(const Ref& other) const noexcept 
//         { 
//             return m_Ptr == other.m_Ptr; 
//         }


// 		/**
// 		 * @brief Inequality operator. Compares the managed objects.
// 		 *
// 		 * @param other The Ref to compare with.
// 		 * @return True if the Refs manage different objects, false otherwise.
// 		 */
// 		bool operator!=(const Ref& other) const noexcept 
//         { 
//             return m_Ptr != other.m_Ptr; 
//         }


// 		/**
// 		 * @brief Equality operator with nullptr. Checks if the Ref is null.
// 		 *
// 		 * @return True if the Ref is null, false otherwise.
// 		 */
// 		bool operator==(std::nullptr_t) const noexcept 
//         { 
//             return m_Ptr == nullptr; 
//         }


// 		/**
// 		 * @brief Inequality operator with nullptr. Checks if the Ref is not null.
// 		 *
// 		 * @return True if the Ref is not null, false otherwise.
// 		 */
// 		bool operator!=(std::nullptr_t) const noexcept 
//         { 
//             return m_Ptr != nullptr; 
//         }


// 	private:

// 		// Helper for SFINAE-based object comparison.
// 		template <typename U>
// 		static auto HasEqualityOperator(int) -> decltype(std::declval<U>() == std::declval<U>(), std::true_type{});

// 		template <typename U>
// 		static std::false_type HasEqualityOperator(...);

// 		template <typename U>
// 		static bool CompareObjectsImpl(const U& a, const U& b, std::true_type)
// 		{
// 		    return a == b;
// 		}

// 		template <typename U>
// 		static bool CompareObjectsImpl(const U& a, const U& b, std::false_type)
// 		{
// 		    return false; // No equality operator available
// 		}

// 	public:

// 		/**
// 		 * @brief Compares the managed objects for object equality.
// 		 *
// 		 * This method compares the objects themselves, not just the pointers.
// 		 * If `T` does not define `operator==`, the comparison returns false.
// 		 *
// 		 * @param other The Ref to compare with.
// 		 * @return True if both objects are equal, false otherwise.
// 		 */
// 		bool equalsObject(const Ref& other) const noexcept
// 		{
// 		    if (m_Ptr == other.m_Ptr)
//             {
//                 return true;
//             }

// 		    if (!m_Ptr || !other.m_Ptr)
//             {
//                 return false;
//             }

// 		    // Use compile-time type trait detection instead of runtime function calls
// 		    using HasEquality = decltype(HasEqualityOperator<T>(0));

// 		    return CompareObjectsImpl(*m_Ptr, *other.m_Ptr, HasEquality{});
// 		 }

// 	private:

// 		T* m_Ptr = nullptr;

// 		void internalAddRef() const noexcept;

// 		void internalRelease() noexcept;

//         /// Grant access to specific classes or functions
//         template <typename U>
//         friend class Ref;

//         template <typename U>
//         friend class WeakRef;
// 	};

// #pragma endregion Ref

//     // ============================================================================================

// #pragma region WeakRef 

// 	/**
// 	 * @brief A weak reference to an object managed by Ref<T>.
// 	 *
// 	 * WeakRef allows observing an object without affecting its lifetime.
// 	 * Unlike Ref<T>, WeakRef does not prevent the object from being destroyed.
// 	 *
// 	 * @tparam T The type of the managed object.
// 	 */
// 	template <typename T>
// 	class WeakRef
// 	{
// 	public:

// 		/**
// 		 * @brief Default constructor creates an empty weak reference.
// 		 */
// 		constexpr WeakRef() noexcept = default;
        

// 		/**
// 		 * @brief Constructor from nullptr creates an empty weak reference.
// 		 */
// 		constexpr WeakRef(std::nullptr_t) noexcept {}


// 		/**
// 		 * @brief Constructor from a Ref<T>. Creates a weak reference to the object managed by ref.
// 		 *
// 		 * @param ref The Ref<T> to observe.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		WeakRef(const Ref<U>& ref) noexcept;


// 		/**
// 		 * @brief Copy constructor.
// 		 *
// 		 * @param other The WeakRef to copy from.
// 		 */
// 		WeakRef(const WeakRef& other) noexcept;


// 		/**
// 		 * @brief Copy constructor with type conversion.
// 		 *
// 		 * @tparam U The type of the other WeakRef.
// 		 * @param other The WeakRef to copy from.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		WeakRef(const WeakRef<U>& other) noexcept;


// 		/**
// 		 * @brief Move constructor.
// 		 *
// 		 * @param other The WeakRef to move from.
// 		 */
// 		WeakRef(WeakRef&& other) noexcept;


// 		/**
// 		 * @brief Move constructor with type conversion.
// 		 *
// 		 * @tparam U The type of the other WeakRef.
// 		 * @param other The WeakRef to move from.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		WeakRef(WeakRef<U>&& other) noexcept;


// 		/**
// 		 * @brief Destructor.
// 		 */
// 		~WeakRef();


//         Internal::ControlBlock<T>* getControlBlock() const noexcept
//         {
//             return m_ControlBlock;
//         }


// 		/**
// 		 * @brief Copy assignment operator.
// 		 *
// 		 * @param other The WeakRef to copy from.
// 		 * @return Reference to this WeakRef.
// 		 */
// 		WeakRef& operator=(const WeakRef& other) noexcept;


// 		/**
// 		 * @brief Copy assignment operator with type conversion.
// 		 *
// 		 * @tparam U The type of the other WeakRef.
// 		 * @param other The WeakRef to copy from.
// 		 * @return Reference to this WeakRef.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		WeakRef& operator=(const WeakRef<U>& other) noexcept;


// 		/**
// 		 * @brief Move assignment operator.
// 		 *
// 		 * @param other The WeakRef to move from.
// 		 * @return Reference to this WeakRef.
// 		 */
// 		WeakRef& operator=(WeakRef&& other) noexcept;


// 		/**
// 		 * @brief Move assignment operator with type conversion.
// 		 *
// 		 * @tparam U The type of the other WeakRef.
// 		 * @param other The WeakRef to move from.
// 		 * @return Reference to this WeakRef.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		WeakRef& operator=(WeakRef<U>&& other) noexcept;


// 		/**
// 		 * @brief Assignment operator from Ref<T>.
// 		 *
// 		 * @param ref The Ref<T> to observe.
// 		 * @return Reference to this WeakRef.
// 		 */
// 		template <typename U, typename = std::enable_if_t<std::is_convertible_v<U*, T*>>>
// 		WeakRef& operator=(const Ref<U>& ref) noexcept;


// 		/**
// 		 * @brief Assignment operator from nullptr.
// 		 *
// 		 * @return Reference to this WeakRef.
// 		 */
// 		WeakRef& operator=(std::nullptr_t) noexcept;


// 		/**
// 		 * @brief Checks if the WeakRef is expired.
// 		 *
// 		 * A WeakRef is expired if the object it points to has been destroyed.
// 		 *
// 		 * @return True if the WeakRef is expired, false otherwise.
// 		 */
// 		bool expired() const noexcept;


// 		/**
// 		 * @brief Attempts to get a strong reference to the object.
// 		 *
// 		 * @return A Ref<T> to the object, or an empty Ref<T> if the object has been destroyed.
// 		 */
// 		Ref<T> lock() const noexcept;


// 		/**
// 		 * @brief Resets the WeakRef.
// 		 */
// 		void reset() noexcept;


// 		/**
// 		 * @brief Gets the reference count of the object.
// 		 *
// 		 * @return The number of Ref<T> instances that share ownership of the object, or 0 if the WeakRef is expired.
// 		 */
// 		U32 useCount() const noexcept;


// 		/**
// 		 * @brief Equality operator.
// 		 *
// 		 * @param other The WeakRef to compare with.
// 		 * @return True if both WeakRefs observe the same object, false otherwise.
// 		 */
// 		bool operator==(const WeakRef& other) const noexcept;


// 		/**
// 		 * @brief Inequality operator.
// 		 *
// 		 * @param other The WeakRef to compare with.
// 		 * @return True if the WeakRefs observe different objects, false otherwise.
// 		 */
// 		bool operator!=(const WeakRef& other) const noexcept;

// 	private:

// 		Internal::ControlBlock<T>* m_ControlBlock = nullptr;

//         // Allow all WeakRef instantiations to access each other's private members
//         template <typename> friend class WeakRef;

// 		/// Allow Ref<T> to access m_ControlBlock
// 		template <typename U>
// 		friend class Ref;
// 	};

// #pragma endregion WeakRef

//     // ============================================================================================

// 	/**
// 	 * @brief Creates a reference-counted object of type T.
// 	 *
// 	 * This function creates a new instance of T and wraps it in a Ref<T>.
// 	 * The type T must inherit from RefCounted.
// 	 *
// 	 * @tparam T The type to create.
// 	 * @tparam Args The types of the arguments to pass to the constructor of T.
// 	 * @param args The arguments to pass to the constructor of T.
// 	 * @return A Ref<T> managing the new object.
// 	 */
// 	template <typename T, typename... Args>
// 	Ref<T> CreateRef(Args&&... args)
// 	{
// 		static_assert(std::is_base_of_v<RefCounted, T>, "Type must inherit from RefCounted");

//         return Ref<T>(new T(std::forward<Args>(args)...));
// 	}


//     /**
// 	 * @brief Increments the reference count for the object.
// 	 *
// 	 * This method is called when a new reference to the object is created.
// 	 * It safely increments the internal reference counter of the pointed object
// 	 * if the pointer is not null.
// 	 *
// 	 * @tparam T The type of the reference-counted object
// 	 * @return void
// 	 */
//     template <typename T>
//     void Ref<T>::internalAddRef() const noexcept
//     {
//         if (m_Ptr)
//         {
//             m_Ptr->incRefCount();
//         }
//     }


//     /**
// 	 * @brief Releases a reference to the object and potentially deletes it.
// 	 *
// 	 * This method decrements the reference count of the pointed object.
// 	 * If the reference count reaches zero, it updates any weak references
// 	 * through the ControlBlockRegistry to indicate that the object is no longer valid,
// 	 * and then deletes the object.
// 	 *
// 	 * The method ensures that:
// 	 * 1. Weak references can detect that the object has been destroyed
// 	 * 2. The object memory is properly freed when no more strong references exist
// 	 * 3. The internal pointer is set to nullptr after the release
// 	 *
// 	 * @tparam T The type of the reference-counted object
// 	 * @return void
// 	 */
//     template <typename T>
//     void Ref<T>::internalRelease() noexcept
//     {
//         if (m_Ptr)
//         {
//             if (m_Ptr->decRefCount() == 0)
//             {
//                 /// Update any weak references before deleting the object
//                 Internal::ControlBlockRegistry<T>::getInstance().removeControlBlock(m_Ptr);

//                 delete m_Ptr;
//             }

//             m_Ptr = nullptr;
//         }
//     }

//     // ============================================================================================

//     /**
// 	 * @brief Constructs a weak reference from a strong reference.
// 	 *
// 	 * This constructor creates a WeakRef that weakly references the same object
// 	 * as the provided strong reference (Ref<U>). The constructor supports proper
// 	 * type conversion through the template parameter U, which must be convertible to T.
// 	 *
// 	 * The implementation:
// 	 * 1. Checks if the provided reference is valid
// 	 * 2. Retrieves or creates a control block for the referenced object
// 	 * 3. Increments the weak reference count in the control block
// 	 *
// 	 * @tparam U The type of the source reference, must be convertible to T
// 	 * @param ref The strong reference to create a weak reference from
// 	 * @note - This constructor enables implicit conversion from Ref<U> to WeakRef<T>
// 	 *       when U is convertible to T
// 	 */
//     template <typename T>
//     template <typename U, typename>
//     WeakRef<T>::WeakRef(const Ref<U>& ref) noexcept
//     {
//         if (ref)
//         {
//             m_ControlBlock = Internal::ControlBlockRegistry<T>::getInstance().getControlBlock(static_cast<T*>(ref.get()));

//             if (m_ControlBlock)
//             {
//                 m_ControlBlock->incWeakCount();
//             }
//         }
//     }


//     /**
//      * @brief Copy constructor for weak references.
//      *
//      * This constructor creates a new WeakRef that weakly references the same object
//      * as the provided source WeakRef. If the source WeakRef is valid (points to a
//      * control block), this constructor:
//      * 1. Copies the control block pointer from the source
//      * 2. Increments the weak reference count in that control block
//      *
//      * @param other The source WeakRef to copy from
//      * @note - This maintains proper reference counting without affecting the
//      *       lifetime of the referenced object
//      */
//     template <typename T>
//     WeakRef<T>::WeakRef(const WeakRef& other) noexcept : 
//         m_ControlBlock(other.m_ControlBlock)
//     {
//         if (m_ControlBlock)
//         {
//             m_ControlBlock->incWeakCount();
//         }
//     }


//     /**
// 	 * @brief Copy conversion constructor for WeakRef objects of different but compatible types.
// 	 *
// 	 * This constructor allows creation of a WeakRef<T> from a WeakRef<U> where U is convertible to T
// 	 * (typically through inheritance relationships). It properly maintains the weak reference counting
// 	 * through the control block system.
// 	 *
// 	 * The implementation:
// 	 * 1. Checks if the source WeakRef has a valid control block
// 	 * 2. Retrieves or creates a control block for T* through the registry using a static_cast
// 	 * 3. Increments the weak reference count if a valid control block is found
// 	 *
// 	 * @tparam U Source type that is convertible to T
// 	 * @param other The source WeakRef<U> to convert from
// 	 * @note - This constructor only participates in overload resolution if U* is convertible to T*
// 	 */
//     template <typename T>
//     template <typename U, typename>
//     WeakRef<T>::WeakRef(const WeakRef<U>& other) noexcept
//     {
//         if (other.m_ControlBlock)
//         {
//             m_ControlBlock = Internal::ControlBlockRegistry<T>::getInstance().getControlBlock(static_cast<T*>(other.m_ControlBlock->getPtr()));

//             if (m_ControlBlock)
//             {
//                 m_ControlBlock->incWeakCount();
//             }
//         }
//     }


//     /**
// 	 * @brief Move constructor for weak references.
// 	 *
// 	 * This constructor creates a new WeakRef by transferring ownership of the control block
// 	 * from the source WeakRef. After the move, the source WeakRef no longer references
// 	 * any object (its control block pointer is set to nullptr).
// 	 *
// 	 * The implementation:
// 	 * 1. Takes ownership of the control block pointer from the source WeakRef
// 	 * 2. Sets the source WeakRef's control block pointer to nullptr to prevent
// 	 *    both instances from managing the same control block
// 	 *
// 	 * Unlike the copy constructor, this constructor doesn't increment the weak reference count
// 	 * since ownership is being transferred rather than shared.
// 	 *
// 	 * @param other The source WeakRef to move from
// 	 */
//     template <typename T>
//     WeakRef<T>::WeakRef(WeakRef&& other) noexcept : 
//         m_ControlBlock(other.m_ControlBlock)
//     {
//         other.m_ControlBlock = nullptr;
//     }


//     /**
// 	 * @brief Move conversion constructor for weak references of different but compatible types.
// 	 *
// 	 * This constructor moves a WeakRef<U> to a WeakRef<T> where U is convertible to T
// 	 * (typically through inheritance relationships). Unlike the regular move constructor,
// 	 * this constructor performs a type conversion which requires finding or creating a
// 	 * control block for the target type.
// 	 *
// 	 * The implementation:
// 	 * 1. Checks if the source WeakRef has a valid control block
// 	 * 2. If valid, retrieves or creates a control block for the target type through the registry
// 	 * 3. Sets the source WeakRef's control block to nullptr to transfer ownership
// 	 * 4. No increment of weak reference count is needed as ownership is transferred
// 	 *
// 	 * @tparam U Source type that is convertible to T
// 	 * @param other The source WeakRef<U> to move from
// 	 * @note - This constructor only participates in overload resolution if U* is convertible to T*
// 	 *       (enforced by the SFINAE template parameter)
// 	 */
//     template <typename T>
//     template <typename U, typename>
//     WeakRef<T>::WeakRef(WeakRef<U>&& other) noexcept
//     {
//         if (other.m_ControlBlock)
//         {
//             m_ControlBlock = Internal::ControlBlockRegistry<T>::getInstance().getControlBlock(static_cast<T*>(other.m_ControlBlock->getPtr()));

//             other.m_ControlBlock = nullptr;
//         }
//     }


//     /**
// 	 * @brief Destructor for the weak reference.
// 	 *
// 	 * This destructor properly cleans up resources associated with the weak reference.
// 	 * When a WeakRef is destroyed, it decrements the weak reference count in the associated
// 	 * control block. If this was the last weak reference and the object has already been
// 	 * destroyed (control block's pointer is null), the control block itself will be deleted.
// 	 *
// 	 * The destruction process ensures that:
// 	 * 1. All weak references are properly tracked
// 	 * 2. Control blocks are cleaned up when no longer needed
// 	 * 3. No memory leaks occur when weak references go out of scope
// 	 */
//     template <typename T>
//     WeakRef<T>::~WeakRef()
//     {
//         if (m_ControlBlock)
//         {
//             m_ControlBlock->decWeakCount();
//         }
//     }


//     /**
// 	 * @brief Copy assignment operator for weak references.
// 	 *
// 	 * This operator assigns the content of another WeakRef to this WeakRef.
// 	 * If this WeakRef is already referencing an object, it decrements the
// 	 * weak reference count in that object's control block. Then it copies
// 	 * the control block pointer from the source WeakRef and increments the
// 	 * weak reference count if the control block is valid.
// 	 *
// 	 * The implementation:
// 	 * 1. Checks for self-assignment to avoid unnecessary operations
// 	 * 2. Decrements the weak reference count in the current control block (if any)
// 	 * 3. Copies the control block pointer from the source WeakRef
// 	 * 4. Increments the weak reference count in the new control block (if valid)
// 	 *
// 	 * @param other The source WeakRef to copy from
// 	 * @return A reference to this WeakRef after the assignment
// 	 * @note - This operator maintains proper reference counting without affecting the
// 	 *       lifetime of the referenced object
// 	 */
//     template <typename T>
//     WeakRef<T>& WeakRef<T>::operator=(const WeakRef& other) noexcept
//     {
//         if (this != &other)
//         {
//             if (m_ControlBlock)
//             {
//                 m_ControlBlock->decWeakCount();
//             }

//             m_ControlBlock = other.m_ControlBlock;

//             if (m_ControlBlock)
//             {
//                 m_ControlBlock->incWeakCount();
//             }
//         }

//         return *this;
//     }


//     /**
// 	 * @brief Copy conversion assignment operator for WeakRef objects of different but compatible types.
// 	 *
// 	 * This operator allows assignment of a WeakRef<U> to a WeakRef<T> where U is convertible to T
// 	 * (typically through inheritance relationships). It properly maintains the weak reference counting
// 	 * through the control block system.
// 	 *
// 	 * The implementation:
// 	 * 1. Decrements the weak reference count for this WeakRef's current control block (if any)
// 	 * 2. Retrieves or creates a control block for the T* pointer obtained by static_casting the U* pointer
// 	 *    from the source WeakRef's control block
// 	 * 3. Increments the weak reference count if a valid control block is found
// 	 *
// 	 * @tparam U Source type that is convertible to T
// 	 * @param other The source WeakRef<U> to assign from
// 	 * @return A reference to this WeakRef after the assignment
// 	 * @note - This operator only participates in overload resolution if U* is convertible to T*
// 	 *       (enforced by the SFINAE template parameter)
// 	 */
//     template <typename T>
//     template <typename U, typename>
//     WeakRef<T>& WeakRef<T>::operator=(const WeakRef<U>& other) noexcept
//     {
//         if (m_ControlBlock)
//         {
//             m_ControlBlock->decWeakCount();
//             m_ControlBlock = nullptr;
//         }

//         if (other.m_ControlBlock)
//         {
//             m_ControlBlock = Internal::ControlBlockRegistry<T>::getInstance().getControlBlock(static_cast<T*>(other.m_ControlBlock->getPtr()));

//             if (m_ControlBlock)
//             {
//                 m_ControlBlock->incWeakCount();
//             }
//         }

//         return *this;
//     }
    

//     /**
// 	 * @brief Move assignment operator for weak references.
// 	 *
// 	 * This operator assigns the content of another WeakRef to this WeakRef through move semantics.
// 	 * Move assignment is more efficient than copy assignment as it transfers ownership of the
// 	 * control block pointer rather than copying it and incrementing reference counts.
// 	 *
// 	 * The implementation:
// 	 * 1. Checks for self-assignment to avoid unnecessary operations
// 	 * 2. Decrements the weak reference count in the current control block (if any)
// 	 * 3. Takes ownership of the control block pointer from the source WeakRef
// 	 * 4. Sets the source WeakRef's control block pointer to nullptr to prevent
// 	 *    both instances from managing the same control block
// 	 *
// 	 * @param other The source WeakRef to move from
// 	 * @return A reference to this WeakRef after the assignment
// 	 */
//     template <typename T>
//     WeakRef<T>& WeakRef<T>::operator=(WeakRef&& other) noexcept
//     {
//         if (this != &other)
//         {
//             if (m_ControlBlock)
//             {
//                 m_ControlBlock->decWeakCount();
//             }

//             m_ControlBlock       = other.m_ControlBlock;
//             other.m_ControlBlock = nullptr;
//         }

//         return *this;
//     }


//     /**
// 	 * @brief Move conversion assignment operator for weak references of different but compatible types.
// 	 *
// 	 * This operator moves a WeakRef<U> to a WeakRef<T> where U is convertible to T
// 	 * (typically through inheritance relationships). Unlike the regular move assignment operator,
// 	 * this operator performs a type conversion which requires finding or creating a
// 	 * control block for the target type.
// 	 *
// 	 * The implementation:
// 	 * 1. Decrements the weak reference count of the current control block (if any)
// 	 * 2. If the source WeakRef has a valid control block, retrieves or creates a control block
// 	 *    for the target type through the registry
// 	 * 3. Sets the source WeakRef's control block to nullptr to prevent both instances
// 	 *    from managing the same control block
// 	 * 4. No increment of weak reference count is needed as ownership is transferred
// 	 *
// 	 * @tparam U Source type that is convertible to T
// 	 * @param other The source WeakRef<U> to move from
// 	 * @return A reference to this WeakRef after the assignment
// 	 * @note - This operator only participates in overload resolution if U* is convertible to T*
// 	 *       (enforced by the SFINAE template parameter)
// 	 */
//     template <typename T>
//     template <typename U, typename>
//     WeakRef<T>& WeakRef<T>::operator=(WeakRef<U>&& other) noexcept
//     {
//         if (m_ControlBlock)
//         {
//             m_ControlBlock->decWeakCount();
//             m_ControlBlock = nullptr;
//         }

//         if (other.m_ControlBlock)
//         {
//             m_ControlBlock = Internal::ControlBlockRegistry<T>::getInstance().getControlBlock(static_cast<T*>(other.m_ControlBlock->getPtr()));
//             other.m_ControlBlock = nullptr;
//         }

//         return *this;
//     }


//     /**
// 	 * @brief Assignment operator that assigns a strong reference to a weak reference.
// 	 *
// 	 * This operator assigns a strong reference (Ref<U>) to this weak reference (WeakRef<T>).
// 	 * It properly maintains weak reference counting through the control block system.
// 	 *
// 	 * The implementation:
// 	 * 1. Decrements the weak reference count in the current control block (if any)
// 	 * 2. Clears the current control block pointer
// 	 * 3. If the source reference is valid, retrieves or creates a control block for the referenced object
// 	 * 4. Increments the weak reference count if a valid control block is found
// 	 *
// 	 * @tparam U Source type that is convertible to T
// 	 * @param ref The source Ref<U> to assign from
// 	 * @return A reference to this WeakRef after the assignment
// 	 * @note - This operator only participates in overload resolution if U* is convertible to T*
// 	 *       (enforced by the SFINAE template parameter)
// 	 */
//     template <typename T>
//     template <typename U, typename>
//     WeakRef<T>& WeakRef<T>::operator=(const Ref<U>& ref) noexcept
//     {
//         if (m_ControlBlock)
//         {
//             m_ControlBlock->decWeakCount();
//             m_ControlBlock = nullptr;
//         }

//         if (ref)
//         {
//             m_ControlBlock = Internal::ControlBlockRegistry<T>::getInstance().getControlBlock(static_cast<T*>(ref.get()));

//             if (m_ControlBlock)
//             {
//                 m_ControlBlock->incWeakCount();
//             }
//         }

//         return *this;
//     }


//     /**
// 	 * @brief Assignment operator for assigning nullptr to a weak reference.
// 	 *
// 	 * This operator allows assigning nullptr to a WeakRef, which effectively
// 	 * resets the weak reference. It decrements the weak reference count in the
// 	 * associated control block (if any) and sets the control block pointer to nullptr.
// 	 *
// 	 * The implementation:
// 	 * 1. Decrements the weak reference count if a valid control block exists
// 	 * 2. Sets the control block pointer to nullptr
// 	 *
// 	 * @param unused Nullptr value (not used in the implementation)
// 	 * @return A reference to this WeakRef after the assignment
// 	 */
//     template <typename T>
//     WeakRef<T>& WeakRef<T>::operator=(std::nullptr_t) noexcept
//     {
//         if (m_ControlBlock)
//         {
//             m_ControlBlock->decWeakCount();
//             m_ControlBlock = nullptr;
//         }

//         return *this;
//     }


//     /**
// 	 * @brief Checks if the object pointed to by the weak reference has been destroyed.
// 	 *
// 	 * This method determines whether the WeakRef is expired by checking if:
// 	 * 1. The control block is null (indicating an empty weak reference), or
// 	 * 2. The pointer stored in the control block is null (indicating the referenced object has been destroyed)
// 	 *
// 	 * A WeakRef becomes expired when the last Ref pointing to the same object is destroyed,
// 	 * which triggers the object's deletion. The control block maintains this information
// 	 * even after the object is gone.
// 	 *
// 	 * @return true if the referenced object has been destroyed or if this is an empty WeakRef
// 	 * @return false if the referenced object is still alive
// 	 */
//     template <typename T>
//     bool WeakRef<T>::expired() const noexcept
//     {
//         return !m_ControlBlock || m_ControlBlock->getPtr() == nullptr;
//     }


//     /**
// 	 * @brief Attempts to convert a weak reference to a strong reference.
// 	 *
// 	 * This method tries to obtain a strong reference (Ref<T>) from the weak reference.
// 	 * If the object of the WeakRef points to is still alive (not expired), it creates
// 	 * and returns a new Ref<T> pointing to that object, which increments the reference
// 	 * count of the object. If the object has been destroyed, it returns an empty Ref<T>.
// 	 *
// 	 * @tparam T The type of the referenced object
// 	 * @return Ref<T> A strong reference to the object if it's still alive, or an empty reference otherwise
// 	 */
//     template <typename T>
//     Ref<T> WeakRef<T>::lock() const noexcept
//     {
//         if (!m_ControlBlock || m_ControlBlock->getPtr() == nullptr) 
//         {
//             return Ref<T>(nullptr);
//         }

//         T* ptr = static_cast<T*>(m_ControlBlock->getPtr());

//         return Ref<T>(ptr);
//     }


//     /**
// 	 * @brief Resets this weak reference to empty state.
// 	 *
// 	 * This method explicitly releases the weak reference to any object it might be pointing to.
// 	 * It decrements the weak reference count in the associated control block, and if this
// 	 * was the last weak reference and the object has already been destroyed, the control block
// 	 * itself will be deleted.
// 	 *
// 	 * After calling reset(), the weak reference will be in an empty state (similar to a
// 	 * default-constructed WeakRef) and will return true for expired() and nullptr for lock().
// 	 *
// 	 * @note - This method is often used to explicitly release resources before the WeakRef
// 	 *       goes out of scope, or to prepare the WeakRef for reuse.
// 	 */
//     template <typename T>
//     void WeakRef<T>::reset() noexcept
//     {
//         if (m_ControlBlock)
//         {
//             m_ControlBlock->decWeakCount();
//             m_ControlBlock = nullptr;
//         }
//     }


//     /**
// 	 * @brief Gets the current number of strong references (Ref<T>) to the object.
// 	 *
// 	 * This method returns the reference count of the object that this WeakRef
// 	 * points to. If the WeakRef is expired (the object has been destroyed) or
// 	 * if it's an empty WeakRef, the method returns 0.
// 	 *
// 	 * This is useful for debugging and testing purposes, or for algorithms that
// 	 * need to make decisions based on the reference count of an object.
// 	 *
// 	 * @tparam T The type of the referenced object
// 	 * @return The number of strong references to the object, or 0 if the WeakRef is expired
// 	 */
//     template <typename T>
//     U32 WeakRef<T>::useCount() const noexcept
//     {
//         if (m_ControlBlock && m_ControlBlock->getPtr())
//         {
//             return m_ControlBlock->getPtr()->getRefCount();
//         }

//         return 0;
//     }


//     /**
// 	 * @brief Equality comparison operator for WeakRef objects.
// 	 *
// 	 * This operator determines if two WeakRef objects reference the same underlying object.
// 	 * The comparison is done in the following order:
// 	 * 1. First checks if both WeakRefs have the same control block pointer (fast path)
// 	 * 2. If control blocks differ, checks if either is nullptr (meaning one reference is empty)
// 	 * 3. Finally compares the actual object pointers stored in the control blocks
// 	 *
// 	 * This enables WeakRef objects to be used in containers that require equality comparison,
// 	 * such as std::set, std::map, or for general comparison operations.
// 	 *
// 	 * @param other The WeakRef to compare with
// 	 * @return true if both WeakRef objects reference the same object or are both empty
// 	 * @return false if the WeakRef objects reference different objects or one is empty and one is not
// 	 */
//     template <typename T>
//     bool WeakRef<T>::operator==(const WeakRef& other) const noexcept
//     {
//         if (m_ControlBlock == other.m_ControlBlock)
//         {
//             return true;
//         }

//         if (!m_ControlBlock || !other.m_ControlBlock)
//         {
//             return false;
//         }

//         return m_ControlBlock->getPtr() == other.m_ControlBlock->getPtr();
//     }


//     /**
// 	 * @brief Inequality comparison operator for WeakRef objects.
// 	 *
// 	 * This operator determines if two WeakRef objects reference different underlying objects.
// 	 * It is implemented by negating the result of the equality operator.
// 	 *
// 	 * Two WeakRef objects are considered not equal if:
// 	 * 1. They reference different objects, or
// 	 * 2. One references an object and the other is empty, or
// 	 * 3. They are both empty but have different control blocks
// 	 *
// 	 * @param other The WeakRef to compare with
// 	 * @return true if the WeakRef objects reference different objects or have different empty states
// 	 * @return false if both WeakRef objects reference the same object or are both empty
// 	 */
//     template <typename T>
//     bool WeakRef<T>::operator!=(const WeakRef& other) const noexcept
//     {
//         return !(*this == other);
//     }


//     /**
// 	 * @brief Constructor that creates a strong reference from a weak reference.
// 	 *
// 	 * This constructor attempts to convert a WeakRef<T> to a Ref<T> by checking if
// 	 * the object pointed to by the weak reference is still alive. If the weak reference
// 	 * is valid (not expired), this constructor creates a new strong reference to the object
// 	 * and increments its reference count. If the weak reference is expired (the object
// 	 * has been destroyed), the constructor creates an empty Ref (m_Ptr = nullptr).
// 	 *
// 	 * The implementation:
// 	 * 1. Retrieves the object pointer from the weak reference's control block if it exists
// 	 * 2. Initializes the internal pointer to the retrieved object or nullptr
// 	 * 3. Calls internalAddRef() to increment the reference count if the pointer is valid
// 	 *
// 	 * @param weak The weak reference to convert to a strong reference
// 	 * @note - This enables safe conversion from WeakRef<T> to Ref<T>, preventing access to destroyed objects
// 	 */
//     template <typename T>
//     inline Ref<T>::Ref(const WeakRef<T>& weak) noexcept
//     {
//         if (weak.getControlBlock() && weak.getControlBlock()->getPtr()) 
//         {
//             m_Ptr = static_cast<T*>(weak.getControlBlock()->getPtr());
//             internalAddRef();
//         } 
//         else 
//         {
//             m_Ptr = nullptr;
//         }
//     }


//     /**
//      * @brief Equality operator for comparing objects through two Ref instances.
//      *
//      * This operator compares the objects managed by two Ref instances for equality.
//      * It first checks if the Ref instances point to the same address (identity comparison).
//      * If not, it checks if both Refs are valid, and if so, compares the objects using
//      * their operator== implementation.
//      *
//      * This operator requires that type T has a valid operator== defined.
//      *
//      * @tparam T The type of objects managed by the Ref instances
//      * @param lhs The left-hand side Ref for comparison
//      * @param rhs The right-hand side Ref for comparison
//      * @return True if both Refs manage equal objects, false otherwise
//      */
//     template <typename T>
//     bool operator==(const Ref<T>& lhs, const Ref<T>& rhs) noexcept
//     {
//         if (lhs.get() == rhs.get())
//         {
//             return true;
//         }

//         if (!lhs || !rhs)
//         {
//             return false;
//         }

//         return *lhs == *rhs;
//     }


//     /**
//      * @brief Inequality operator for comparing objects through two Ref instances.
//      *
//      * This operator compares the objects managed by two Ref instances for inequality.
//      * It is implemented by negating the result of the equality operator.
//      *
//      * This operator requires that type T has a valid operator== defined.
//      *
//      * @tparam T The type of objects managed by the Ref instances
//      * @param lhs The left-hand side Ref for comparison
//      * @param rhs The right-hand side Ref for comparison
//      * @return True if the Refs manage different objects, false if they manage equal objects
//      */
//     template <typename T>
//     bool operator!=(const Ref<T>& lhs, const Ref<T>& rhs) noexcept
//     {
//         return !(lhs == rhs);
//     }


//     /**
//      * @brief Equality operator for comparing objects through two WeakRef instances.
//      *
//      * This operator compares the objects managed by two WeakRef instances for equality.
//      * It first checks if both WeakRefs are not expired and refer to valid objects.
//      * If so, it locks the WeakRefs to obtain strong references (Refs) and compares
//      * the objects using the operator== implementation for Ref<T>.
//      *
//      * This operator requires that type T has a valid operator== defined.
//      *
//      * @tparam T The type of objects managed by the WeakRef instances
//      * @param lhs The left-hand side WeakRef for comparison
//      * @param rhs The right-hand side WeakRef for comparison
//      * @return True if both WeakRefs manage equal objects, false otherwise
//      */
//     template <typename T>
//     bool operator==(const WeakRef<T>& lhs, const WeakRef<T>& rhs) noexcept
//     {
//         if (lhs.expired() || rhs.expired())
//         {
//             return false;
//         }

//         auto lhsLocked = lhs.lock();
//         auto rhsLocked = rhs.lock();

//         return lhsLocked == rhsLocked;
//     }


//     /**
//      * @brief Inequality operator for comparing objects through two WeakRef instances.
//      *
//      * This operator compares the objects managed by two WeakRef instances for inequality.
//      * It is implemented by negating the result of the equality operator.
//      *
//      * This operator requires that type T has a valid operator== defined.
//      *
//      * @tparam T The type of objects managed by the WeakRef instances
//      * @param lhs The left-hand side WeakRef for comparison
//      * @param rhs The right-hand side WeakRef for comparison
//      * @return True if the WeakRefs manage different objects, false if they manage equal objects
//      */
//     template <typename T>
//     bool operator!=(const WeakRef<T>& lhs, const WeakRef<T>& rhs) noexcept
//     {
//         return !(lhs == rhs);
//     }

//     // ============================================================================================

//     /**
// 	 * @brief Explicit template instantiation for Ref<RefCounted>
// 	 *
// 	 * This explicit instantiation ensures that the compiler generates all the code
// 	 * for Ref<RefCounted> at this point, making it available to all translation units
// 	 * that include this header without having to recompile the template for each use.
// 	 *
// 	 * RefCounted is the base class for all reference-counted objects in the system,
// 	 * so this instantiation is particularly important for the smart pointer system.
// 	 */
//     template class Ref<RefCounted>;

//     /**
// 	 * @brief Explicit template instantiation for WeakRef<RefCounted>
// 	 *
// 	 * This explicit instantiation ensures that the compiler generates all the code
// 	 * for WeakRef<RefCounted> at this point, making it available to all translation units
// 	 * that include this header without having to recompile the template for each use.
// 	 *
// 	 * Weak references to RefCounted objects allow tracking objects without preventing their
// 	 * deletion when all strong references (Ref<T>) are gone, which is essential for
// 	 * breaking reference cycles and implementing observer patterns.
// 	 */
//     template class WeakRef<RefCounted>;
// }
















// namespace Vy
// {
//     namespace PtrUtil
//     {
//         bool isLiveReference(void* ptr);
//         void addToLiveReferences(void* ptr);
//         void removeFromLiveReferences(void* ptr);
//     }

//     class RefCounted
//     {
//     public:
//         RefCounted() : 
//             m_RefCount(0) 
//         {
//         }

//         virtual ~RefCounted() = default;

//         RefCounted(const RefCounted&)            = delete;
//         RefCounted& operator=(const RefCounted&) = delete;
//         RefCounted(RefCounted&&)                 = delete;
//         RefCounted& operator=(RefCounted&&)      = delete;

//         void incRefCount() const 
//         {
//             m_RefCount.fetch_add(1, std::memory_order_relaxed);
//         }

//         void decRefCount() const 
//         {
//             m_RefCount.fetch_sub(1, std::memory_order_acq_rel);
//         }

//         U32 getRefCount() const 
//         {
//             return m_RefCount.load(std::memory_order_relaxed);
//         }

//     private:
//         mutable AtomicU32 m_RefCount;
//     };



//     template<typename DerivedType>
//     class Ref 
//     {

//     public:
//         Ref() noexcept : 
//             m_Ptr(nullptr) 
//         {
//         }

//         Ref(std::nullptr_t) noexcept : 
//             m_Ptr(nullptr) 
//         {
//         }

//         explicit Ref(DerivedType* ptr) noexcept : 
//             m_Ptr(ptr) 
//         {
//             static_assert(std::is_base_of_v< RefCounted, DerivedType >, "DerivedType must derive from RefCounted");

//             addRef();
//         }

//         Ref(const Ref& other) noexcept : 
//             m_Ptr(other.m_Ptr) 
//         {
//             addRef();
//         }

//         Ref(Ref&& other) noexcept : 
//             m_Ptr(other.m_Ptr) 
//         {
//             other.m_Ptr = nullptr;
//         }

//         template< typename U, typename = std::enable_if_t< std::is_convertible_v< U*, DerivedType* > > >
//         Ref(const Ref< U >& other) noexcept : 
//             m_Ptr(other.get()) 
//         {
//             addRef();
//         }

//         ~Ref() 
//         {
//             release();
//         }

//         Ref& operator=(const Ref& other) noexcept 
//         {
//             Ref(other).swap(*this);
//             return *this;
//         }

//         Ref& operator=(Ref&& other) noexcept 
//         {
//             Ref(std::move(other)).swap(*this);
//             return *this;
//         }

//         Ref& operator=(std::nullptr_t) noexcept 
//         {
//             reset();
//             return *this;
//         }

//         void reset(DerivedType* ptr = nullptr) noexcept 
//         {
//             Ref(ptr).swap(*this);
//         }

//         void swap(Ref& other) noexcept 
//         {
//             std::swap(m_Ptr, other.m_Ptr);
//         }

//         DerivedType* get()        const noexcept { return m_Ptr; }
//         DerivedType& operator*()  const noexcept { assert(m_Ptr); return *m_Ptr; }
//         DerivedType* operator->() const noexcept { assert(m_Ptr); return m_Ptr; }

//         explicit operator bool()  const noexcept { return m_Ptr != nullptr; }

//         bool operator==(const Ref& other) const noexcept { return m_Ptr == other.m_Ptr; }
//         bool operator!=(const Ref& other) const noexcept { return m_Ptr != other.m_Ptr; }
//         bool operator< (const Ref& other) const noexcept { return m_Ptr <  other.m_Ptr; }

//     private:
//         void addRef()
//         {
//             if (m_Ptr)
//             {
//                 m_Ptr->incRefCount();
//                 PtrUtil::addToLiveReferences((void*)m_Ptr);
//             }
//         }

//         void release()
//         {
//             if (m_Ptr)
//             {
//                 m_Ptr->decRefCount();

//                 if (m_Ptr->getRefCount() == 0)
//                 {
//                     delete m_Ptr;
//                     PtrUtil::removeFromLiveReferences((void*)m_Ptr);
//                     m_Ptr = nullptr;
//                 }
//             }
//         }

//         DerivedType* m_Ptr;
//     };

//     template< typename T, typename... TArgs >
//     Ref< T > MakeRef(TArgs&&... args) 
//     {
//         return Ref< T >(new T(std::forward< TArgs >(args)...));
//     }

//     template< typename T, typename U >
//     Ref< T > StaticCast(const Ref< U >& ptr) 
//     {
//         return Ref< T >(static_cast<T*>(ptr.get()));
//     }

//     template< typename T, typename U >
//     Ref< T > DynamicCast(const Ref< U >& ptr) 
//     {
//         return Ref< T >(dynamic_cast<T*>(ptr.get()));
//     }

//     template< typename T >
//     class Weak
//     {
//     public:
//         Weak() = default;
//         Weak(Ref< T > arc)
//         {
//             m_Ptr = arc.get();
//         }
//         Weak(T* ptr)
//         {
//             m_Ptr = ptr;
//         }

//         T* operator->() { return m_Ptr; }
//         const T* operator->() const { return m_Ptr; }

//         T& operator*() { return *m_Ptr; }
//         const T& operator*() const { return *m_Ptr; }

//         Ref<T> lock() const noexcept 
//         {
//             if (PtrUtil::isLiveReference(m_Ptr))
//             {
//                 return Ref< T >(m_Ptr);
//             }

//             return nullptr;
//         }

//         bool valid() const { return m_Ptr ? PtrUtil::isLiveReference(m_Ptr) : false; }
//         operator bool() const { return valid(); }

//     private:
//         T* m_Ptr = nullptr;
//     };
// }