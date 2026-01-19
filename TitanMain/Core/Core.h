#pragma once

// Titan::Core - Core Engine Systems
// Contains fundamental engine components like Application, Logger, Window

#include <memory>
#include <string>
#include <functional>
#include "Object.h"

namespace Titan
{
    namespace Core
    {
        class Application : public Object
        {
        public:
            Application();
            virtual ~Application();

            void Run();
            void Close();
            bool IsRunning() const;

            virtual void OnUpdate(float deltaTime) {}
            virtual void OnRender() {}
            virtual void OnEvent() {} // TODO: Define event system

        protected:
            bool m_IsRunning = false;

        private:
            void Initialize();
            void Shutdown();
        };

        class Logger
        {
        public:
            enum class Level
            {
                Trace,
                Debug,
                Info,
                Warning,
                Error,
                Fatal
            };

            static void Initialize();
            static void Shutdown();

            static void Log(Level level, const std::string& message);
            static void LogTrace(const std::string& message);
            static void LogDebug(const std::string& message);
            static void LogInfo(const std::string& message);
            static void LogWarning(const std::string& message);
            static void LogError(const std::string& message);
            static void LogFatal(const std::string& message);
        };

        class Window
        {
        public:
            struct Properties
            {
                std::string Title;
                uint32_t Width;
                uint32_t Height;
                bool VSync;
                bool Fullscreen;
                bool Resizable;
            };

            static std::unique_ptr<Window> Create(const Properties& props = {});

            virtual ~Window() = default;

            virtual void Update() = 0;
            virtual void Present() = 0;

            virtual uint32_t GetWidth() const = 0;
            virtual uint32_t GetHeight() const = 0;
            virtual void* GetNativeWindow() const = 0;

            virtual void SetVSync(bool enabled) = 0;
            virtual bool IsVSync() const = 0;

            // Event callbacks
            std::function<void()> OnClose;
            std::function<void(uint32_t, uint32_t)> OnResize;
        };

        namespace Memory
        {
            void* Allocate(size_t size);
            void Deallocate(void* ptr);
            void* Reallocate(void* ptr, size_t newSize);

            template<typename T, typename... Args>
            T* New(Args&&... args)
            {
                return new T(std::forward<Args>(args)...);
            }

            template<typename T>
            void Delete(T* ptr)
            {
                delete ptr;
            }
        }

    } // namespace Core

} // namespace Titan