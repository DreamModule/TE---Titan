#pragma once

// Titan::Core::Archive - Serialization system
// Simplified version inspired by UE FArchive

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace Titan
{
    namespace Core
    {
        // Archive flags
        enum class ArchiveFlags : uint32_t
        {
            None        = 0,
            Loading     = 1 << 0,  // Archive is loading data
            Saving      = 1 << 1,  // Archive is saving data
            Binary      = 1 << 2,  // Binary serialization
            Text        = 1 << 3,  // Text serialization
            Persistent  = 1 << 4,  // Archive is persistent (file-based)
            Volatile    = 1 << 5,  // Archive is volatile (memory-based)
        };

        inline ArchiveFlags operator|(ArchiveFlags a, ArchiveFlags b)
        {
            return static_cast<ArchiveFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
        }

        inline ArchiveFlags operator&(ArchiveFlags a, ArchiveFlags b)
        {
            return static_cast<ArchiveFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
        }

        // Base archive class
        class Archive
        {
        public:
            Archive(ArchiveFlags flags = ArchiveFlags::None)
                : Flags(flags) {}
            virtual ~Archive() = default;

            // State queries
            bool IsLoading() const { return (Flags & ArchiveFlags::Loading) != ArchiveFlags::None; }
            bool IsSaving() const { return (Flags & ArchiveFlags::Saving) != ArchiveFlags::None; }
            bool IsBinary() const { return (Flags & ArchiveFlags::Binary) != ArchiveFlags::None; }
            bool IsText() const { return (Flags & ArchiveFlags::Text) != ArchiveFlags::None; }
            bool IsPersistent() const { return (Flags & ArchiveFlags::Persistent) != ArchiveFlags::None; }

            // Position control
            virtual void Seek(int64_t position) = 0;
            virtual int64_t Tell() const = 0;
            virtual int64_t TotalSize() const = 0;

            // Basic serialization operators
            virtual Archive& operator<<(bool& value) = 0;
            virtual Archive& operator<<(int8_t& value) = 0;
            virtual Archive& operator<<(uint8_t& value) = 0;
            virtual Archive& operator<<(int16_t& value) = 0;
            virtual Archive& operator<<(uint16_t& value) = 0;
            virtual Archive& operator<<(int32_t& value) = 0;
            virtual Archive& operator<<(uint32_t& value) = 0;
            virtual Archive& operator<<(int64_t& value) = 0;
            virtual Archive& operator<<(uint64_t& value) = 0;
            virtual Archive& operator<<(float& value) = 0;
            virtual Archive& operator<<(double& value) = 0;
            virtual Archive& operator<<(std::string& value) = 0;

            // Array serialization helpers
            template<typename T>
            void SerializeArray(std::vector<T>& array)
            {
                uint32_t size = static_cast<uint32_t>(array.size());
                *this << size;

                if (IsLoading())
                {
                    array.resize(size);
                }

                for (auto& element : array)
                {
                    *this << element;
                }
            }

            // Object serialization
            void SerializeObject(Object*& object);
            void SerializeObjectPtr(ObjectPtr<Object>& objectPtr);

        protected:
            ArchiveFlags Flags;
        };

        // Memory archive - for in-memory serialization
        class MemoryArchive : public Archive
        {
        public:
            MemoryArchive(bool loading = false);
            MemoryArchive(const std::vector<uint8_t>& data, bool loading = true);

            virtual void Seek(int64_t position) override;
            virtual int64_t Tell() const override;
            virtual int64_t TotalSize() const override;

            virtual Archive& operator<<(bool& value) override;
            virtual Archive& operator<<(int8_t& value) override;
            virtual Archive& operator<<(uint8_t& value) override;
            virtual Archive& operator<<(int16_t& value) override;
            virtual Archive& operator<<(uint16_t& value) override;
            virtual Archive& operator<<(int32_t& value) override;
            virtual Archive& operator<<(uint32_t& value) override;
            virtual Archive& operator<<(int64_t& value) override;
            virtual Archive& operator<<(uint64_t& value) override;
            virtual Archive& operator<<(float& value) override;
            virtual Archive& operator<<(double& value) override;
            virtual Archive& operator<<(std::string& value) override;

            const std::vector<uint8_t>& GetData() const { return Data; }
            std::vector<uint8_t>& GetData() { return Data; }

        private:
            std::vector<uint8_t> Data;
            size_t Position = 0;
        };

        // File archive - for file-based serialization
        class FileArchive : public Archive
        {
        public:
            FileArchive(const std::string& filename, bool loading);
            ~FileArchive();

            virtual void Seek(int64_t position) override;
            virtual int64_t Tell() const override;
            virtual int64_t TotalSize() const override;

            virtual Archive& operator<<(bool& value) override;
            virtual Archive& operator<<(int8_t& value) override;
            virtual Archive& operator<<(uint8_t& value) override;
            virtual Archive& operator<<(int16_t& value) override;
            virtual Archive& operator<<(uint16_t& value) override;
            virtual Archive& operator<<(int32_t& value) override;
            virtual Archive& operator<<(uint32_t& value) override;
            virtual Archive& operator<<(int64_t& value) override;
            virtual Archive& operator<<(uint64_t& value) override;
            virtual Archive& operator<<(float& value) override;
            virtual Archive& operator<<(double& value) override;
            virtual Archive& operator<<(std::string& value) override;

            bool IsOpen() const { return FileHandle != nullptr; }

        private:
            void* FileHandle = nullptr; // Platform-specific file handle
        };

    } // namespace Core

} // namespace Titan