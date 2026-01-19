#pragma once

// Titan::Core::Object - Base object system
// Inspired by UE UObject system, simplified for our needs

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Titan
{
    namespace Core
    {
        // Forward declarations
        class Class;
        class Object;

        // Object flags - simplified version of UE's EObjectFlags
        enum class ObjectFlags : uint32_t
        {
            None                    = 0,
            Public                  = 1 << 0,   // Can be referenced by external packages
            Standalone             = 1 << 1,   // Keep object around for editing even if unreferenced
            MarkAsNative           = 1 << 2,   // Object is native to engine
            Transient              = 1 << 3,   // Object is not saved to disk
            MarkAsRootSet          = 1 << 4,   // Object is in root set, do not garbage collect
            NeedInitialization     = 1 << 5,   // Object needs to be initialized
            BeginDestroyed         = 1 << 6,   // Object is being destroyed
            FinishDestroyed        = 1 << 7,   // Object has been destroyed
            BeingRegenerated       = 1 << 8,   // Object is being regenerated
            DefaultSubObject       = 1 << 9,   // Object is a default subobject
            WasLoaded              = 1 << 10,  // Object was loaded
            TextExportTransient    = 1 << 11,  // Object shouldn't be exported to text
            LoadCompleted          = 1 << 12,  // Object loading is complete
            InitializedProps       = 1 << 13,  // Properties have been initialized
            ConstructedObject      = 1 << 14,  // Object has been constructed
        };

        inline ObjectFlags operator|(ObjectFlags a, ObjectFlags b)
        {
            return static_cast<ObjectFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
        }

        inline ObjectFlags operator&(ObjectFlags a, ObjectFlags b)
        {
            return static_cast<ObjectFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
        }

        inline ObjectFlags operator~(ObjectFlags a)
        {
            return static_cast<ObjectFlags>(~static_cast<uint32_t>(a));
        }

        inline ObjectFlags& operator|=(ObjectFlags& a, ObjectFlags b)
        {
            a = a | b;
            return a;
        }

        inline ObjectFlags& operator&=(ObjectFlags& a, ObjectFlags b)
        {
            a = a & b;
            return a;
        }

        // Base object class - similar to UObjectBase
        class ObjectBase
        {
        public:
            ObjectBase();
            virtual ~ObjectBase();

            // Basic properties
            const std::string& GetName() const { return Name; }
            void SetName(const std::string& name) { Name = name; }

            Class* GetClass() const { return ClassPrivate; }
            Object* GetOuter() const { return OuterPrivate; }

            ObjectFlags GetFlags() const { return ObjectFlagsPrivate; }
            void SetFlags(ObjectFlags flags) { ObjectFlagsPrivate = flags; }
            void AddFlags(ObjectFlags flags) { ObjectFlagsPrivate |= flags; }
            void RemoveFlags(ObjectFlags flags) { ObjectFlagsPrivate &= ~flags; }
            bool HasFlags(ObjectFlags flags) const { return (ObjectFlagsPrivate & flags) != ObjectFlags::None; }

            // Memory management
            virtual void BeginDestroy();
            virtual void FinishDestroy();
            bool IsPendingKill() const { return HasFlags(ObjectFlags::BeginDestroyed); }

            // Reference counting
            void AddRef();
            void Release();
            int32_t GetRefCount() const { return RefCount.load(); }

        protected:
            std::string Name;
            Class* ClassPrivate = nullptr;
            Object* OuterPrivate = nullptr;
            ObjectFlags ObjectFlagsPrivate = ObjectFlags::None;
            std::atomic<int32_t> RefCount{0};
        };

        // Main object class - similar to UObject
        class Object : public ObjectBase
        {
        public:
            Object();
            virtual ~Object();

            // Object lifecycle
            virtual void PostInitProperties() {}
            virtual void BeginPlay() {}
            virtual void Tick(float deltaTime) {}
            virtual void EndPlay() {}

            // Serialization
            virtual void Serialize(class Archive& archive) {}

            // Property access
            template<typename T>
            T* GetProperty(const std::string& name);

            template<typename T>
            void SetProperty(const std::string& name, const T& value);

            // Utility functions
            std::string GetFullName() const;
            std::string GetPathName() const;

            // Static functions
            static Object* CreateObject(Class* objectClass, Object* outer = nullptr, const std::string& name = "");
            static void DestroyObject(Object* object);

            template<typename T, typename... Args>
            static T* CreateObject(Object* outer = nullptr, const std::string& name = "")
            {
                return static_cast<T*>(CreateObject(T::StaticClass(), outer, name));
            }
        };

        // Class system - simplified version of UClass
        class Class
        {
        public:
            Class(const std::string& name, Class* superClass = nullptr);
            virtual ~Class();

            const std::string& GetName() const { return Name; }
            Class* GetSuperClass() const { return SuperClass; }

            // Object creation
            virtual Object* CreateObject(Object* outer = nullptr, const std::string& name = "");

            // Reflection-like functions
            virtual std::vector<std::string> GetPropertyNames() const { return {}; }
            virtual bool HasProperty(const std::string& name) const { return false; }

        protected:
            std::string Name;
            Class* SuperClass = nullptr;
        };

        // Object registry - simplified FUObjectArray
        class ObjectRegistry
        {
        public:
            static ObjectRegistry& Get();

            void RegisterObject(Object* object);
            void UnregisterObject(Object* object);

            Object* FindObject(const std::string& name) const;
            std::vector<Object*> GetObjectsOfClass(Class* objectClass) const;

            size_t GetObjectCount() const { return Objects.size(); }

        private:
            ObjectRegistry() = default;
            ~ObjectRegistry() = default;

            std::unordered_map<std::string, Object*> Objects;
            std::unordered_map<Class*, std::vector<Object*>> ObjectsByClass;
        };

        // Smart pointer for objects - simplified TObjectPtr
        template<typename T>
        class ObjectPtr
        {
        public:
            ObjectPtr() : Ptr(nullptr) {}
            ObjectPtr(T* ptr) : Ptr(ptr) { AddRef(); }
            ObjectPtr(const ObjectPtr& other) : Ptr(other.Ptr) { AddRef(); }
            ObjectPtr(ObjectPtr&& other) noexcept : Ptr(other.Ptr) { other.Ptr = nullptr; }

            ~ObjectPtr() { Release(); }

            ObjectPtr& operator=(const ObjectPtr& other)
            {
                if (this != &other)
                {
                    Release();
                    Ptr = other.Ptr;
                    AddRef();
                }
                return *this;
            }

            ObjectPtr& operator=(ObjectPtr&& other) noexcept
            {
                if (this != &other)
                {
                    Release();
                    Ptr = other.Ptr;
                    other.Ptr = nullptr;
                }
                return *this;
            }

            T* operator->() const { return Ptr; }
            T& operator*() const { return *Ptr; }

            operator bool() const { return Ptr != nullptr; }
            bool operator!() const { return Ptr == nullptr; }

            T* Get() const { return Ptr; }

            bool IsValid() const { return Ptr != nullptr; }

        private:
            void AddRef()
            {
                if (Ptr)
                {
                    Ptr->AddRef();
                }
            }

            void Release()
            {
                if (Ptr)
                {
                    Ptr->Release();
                }
            }

            T* Ptr;
        };

        // Helper macros for class registration
        #define TITAN_CLASS(ClassName, SuperClass) \
            class ClassName : public SuperClass { \
            public: \
                static Class* StaticClass() { \
                    static Class s_Class(#ClassName, SuperClass::StaticClass()); \
                    return &s_Class; \
                } \
                virtual Class* GetClass() const override { return StaticClass(); } \
            };

    } // namespace Core

} // namespace Titan