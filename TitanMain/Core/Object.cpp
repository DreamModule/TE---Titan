#include "Object.h"
#include <algorithm>

namespace Titan
{
    namespace Core
    {
        // ObjectBase implementation
        ObjectBase::ObjectBase()
        {
            RefCount.store(1); // Start with 1 reference
        }

        ObjectBase::~ObjectBase()
        {
            // Ensure we're not being destroyed while still referenced
            // In a real implementation, this would be handled by GC
        }

        void ObjectBase::BeginDestroy()
        {
            AddFlags(ObjectFlags::BeginDestroyed);
        }

        void ObjectBase::FinishDestroy()
        {
            AddFlags(ObjectFlags::FinishDestroyed);
        }

        void ObjectBase::AddRef()
        {
            RefCount.fetch_add(1);
        }

        void ObjectBase::Release()
        {
            if (RefCount.fetch_sub(1) == 1)
            {
                // Last reference, object can be destroyed
                delete this;
            }
        }

        // Object implementation
        Object::Object()
        {
            // Initialize basic object properties
            AddFlags(ObjectFlags::NeedInitialization);
        }

        Object::~Object()
        {
            // Cleanup object
        }

        Object* Object::CreateObject(Class* objectClass, Object* outer, const std::string& name)
        {
            if (!objectClass)
                return nullptr;

            Object* newObject = objectClass->CreateObject(outer, name);
            if (newObject)
            {
                ObjectRegistry::Get().RegisterObject(newObject);
                newObject->PostInitProperties();
            }
            return newObject;
        }

        void Object::DestroyObject(Object* object)
        {
            if (!object)
                return;

            object->BeginDestroy();
            ObjectRegistry::Get().UnregisterObject(object);
            object->FinishDestroy();
            object->Release();
        }

        std::string Object::GetFullName() const
        {
            std::string result = Name;
            if (OuterPrivate)
            {
                result = OuterPrivate->GetFullName() + "." + result;
            }
            return result;
        }

        std::string Object::GetPathName() const
        {
            std::string result = Name;
            if (OuterPrivate)
            {
                result = OuterPrivate->GetPathName() + "/" + result;
            }
            return result;
        }

        // Class implementation
        Class::Class(const std::string& name, Class* superClass)
            : Name(name), SuperClass(superClass)
        {
        }

        Class::~Class()
        {
        }

        Object* Class::CreateObject(Object* outer, const std::string& name)
        {
            // Base implementation - derived classes should override
            // In a real implementation, this would use reflection or factory pattern
            return nullptr;
        }

        // ObjectRegistry implementation
        ObjectRegistry& ObjectRegistry::Get()
        {
            static ObjectRegistry instance;
            return instance;
        }

        void ObjectRegistry::RegisterObject(Object* object)
        {
            if (!object)
                return;

            std::string name = object->GetFullName();
            Objects[name] = object;

            Class* objClass = object->GetClass();
            if (objClass)
            {
                ObjectsByClass[objClass].push_back(object);
            }
        }

        void ObjectRegistry::UnregisterObject(Object* object)
        {
            if (!object)
                return;

            std::string name = object->GetFullName();
            Objects.erase(name);

            Class* objClass = object->GetClass();
            if (objClass)
            {
                auto& classObjects = ObjectsByClass[objClass];
                classObjects.erase(
                    std::remove(classObjects.begin(), classObjects.end(), object),
                    classObjects.end()
                );
            }
        }

        Object* ObjectRegistry::FindObject(const std::string& name) const
        {
            auto it = Objects.find(name);
            return it != Objects.end() ? it->second : nullptr;
        }

        std::vector<Object*> ObjectRegistry::GetObjectsOfClass(Class* objectClass) const
        {
            auto it = ObjectsByClass.find(objectClass);
            return it != ObjectsByClass.end() ? it->second : std::vector<Object*>{};
        }

    } // namespace Core

} // namespace Titan