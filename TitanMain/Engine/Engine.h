#pragma once

// Titan::Engine - Main Engine Systems
// Core engine functionality: lifecycle, subsystems, time, resources

#include <memory>
#include <vector>
#include <unordered_map>

namespace Titan
{
    namespace Engine
    {
        class Subsystem;

        class Engine
        {
        public:
            static Engine& GetInstance();

            void Initialize();
            void Shutdown();
            void Run();

            template<typename T>
            void AddSubsystem()
            {
                static_assert(std::is_base_of<Subsystem, T>::value, "T must inherit from Subsystem");
                auto subsystem = std::make_unique<T>();
                m_Subsystems.push_back(std::move(subsystem));
            }

            template<typename T>
            T* GetSubsystem()
            {
                for (auto& subsystem : m_Subsystems)
                {
                    if (auto ptr = dynamic_cast<T*>(subsystem.get()))
                        return ptr;
                }
                return nullptr;
            }

            float GetDeltaTime() const { return m_DeltaTime; }
            float GetTotalTime() const { return m_TotalTime; }

            bool IsInitialized() const { return m_IsInitialized; }

        private:
            Engine() = default;
            ~Engine() = default;
            Engine(const Engine&) = delete;
            Engine& operator=(const Engine&) = delete;

            void Update(float deltaTime);
            void Render();

            std::vector<std::unique_ptr<Subsystem>> m_Subsystems;
            bool m_IsInitialized = false;
            bool m_IsRunning = false;

            float m_DeltaTime = 0.0f;
            float m_TotalTime = 0.0f;
            uint64_t m_LastTime = 0;
        };

        class Subsystem
        {
        public:
            Subsystem() = default;
            virtual ~Subsystem() = default;

            virtual void Initialize() = 0;
            virtual void Shutdown() = 0;
            virtual void Update(float deltaTime) = 0;

            virtual const char* GetName() const = 0;
        };

        class TimeSubsystem : public Subsystem
        {
        public:
            void Initialize() override;
            void Shutdown() override;
            void Update(float deltaTime) override;
            const char* GetName() const override { return "TimeSubsystem"; }

            float GetDeltaTime() const { return m_DeltaTime; }
            float GetTotalTime() const { return m_TotalTime; }
            uint64_t GetFrameCount() const { return m_FrameCount; }

        private:
            float m_DeltaTime = 0.0f;
            float m_TotalTime = 0.0f;
            uint64_t m_FrameCount = 0;
            uint64_t m_LastTime = 0;
        };

        class ResourceManager : public Subsystem
        {
        public:
            void Initialize() override;
            void Shutdown() override;
            void Update(float deltaTime) override;
            const char* GetName() const override { return "ResourceManager"; }

            template<typename T>
            std::shared_ptr<T> LoadResource(const std::string& path)
            {
                // TODO: Implement resource loading
                return nullptr;
            }

            template<typename T>
            void UnloadResource(const std::shared_ptr<T>& resource)
            {
                // TODO: Implement resource unloading
            }

        private:
            std::unordered_map<std::string, std::shared_ptr<void>> m_Resources;
        };

    } // namespace Engine

} // namespace Titan