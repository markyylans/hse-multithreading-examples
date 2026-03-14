#pragma once

#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <format>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>

namespace ipc {

inline constexpr uint32_t kProtocolVersion = 1;

struct MessageHeader {
    uint32_t type;
    size_t length;
};

struct QueueMeta {
    uint32_t version;
    std::atomic<uint64_t> head;
    std::atomic<uint64_t> tail;
    uint8_t buffer[];
};

inline size_t get_shm_size(int fd) {
    struct stat st;
    if (fstat(fd, &st) == -1) {
        throw std::runtime_error(std::format("fstat failed: {}", strerror(errno)));
    }
    return st.st_size;
}

inline void* map_shm(int fd, size_t size) {
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        throw std::runtime_error(std::format("mmap failed: {}", strerror(errno)));
    }
    return ptr;
}

inline void validate_version(const QueueMeta* meta) {
    if (meta->version != kProtocolVersion) {
        throw std::runtime_error("protocol version mismatch");
    }
}

class ProducerNode {
public:
    ProducerNode(const char* path, size_t queue_size)
        : path_(path), queue_size_(queue_size), buffer_capacity_(queue_size - sizeof(QueueMeta)) {
        shm_fd_ = shm_open(path, O_RDWR, 0666);

        if (shm_fd_ != -1) {
            AttachToExisting();
            return;
        }

        if (errno != ENOENT) {
            throw std::runtime_error(std::format("shm_open failed: {}", strerror(errno)));
        }

        CreateNew(queue_size);
    }

    ~ProducerNode() {
        if (data_) {
            munmap(data_, queue_size_);
        }
        if (shm_fd_ >= 0) {
            close(shm_fd_);
        }
    }

    ProducerNode(const ProducerNode&) = delete;
    ProducerNode& operator=(const ProducerNode&) = delete;

    bool Send(uint32_t type, std::span<const std::byte> payload) {
        auto msg_size = sizeof(MessageHeader) + payload.size();
        if (msg_size > buffer_capacity_) {
            return false;
        }

        while (true) {
            auto head = meta_->head.load(std::memory_order_acquire);
            auto tail = meta_->tail.load(std::memory_order_relaxed);

            if (tail - head + msg_size > buffer_capacity_) {
                return false;
            }

            if (meta_->tail.compare_exchange_weak( 
                    tail,
                    tail + msg_size,
                    std::memory_order_release,
                    std::memory_order_relaxed
                )
            ) {
                WriteMessage(tail, type, payload);
                return true;
            }
        }
    }

private:
    void AttachToExisting() {
        queue_size_ = get_shm_size(shm_fd_);
        buffer_capacity_ = queue_size_ - sizeof(QueueMeta);

        data_ = reinterpret_cast<char*>(map_shm(shm_fd_, queue_size_));
        meta_ = reinterpret_cast<QueueMeta*>(data_);

        try {
            validate_version(meta_);
        }
        catch (...) {
            munmap(data_, queue_size_);
            close(shm_fd_);
            data_ = nullptr;
            shm_fd_ = -1;
            throw;
        }
    }

    void CreateNew(size_t queue_size) {
        shm_unlink(path_);
        shm_fd_ = shm_open(path_, O_CREAT | O_RDWR, 0666);
        if (shm_fd_ == -1) {
            throw std::runtime_error(std::format("shm_open failed: {}", strerror(errno)));
        }

        if (ftruncate(shm_fd_, queue_size) == -1) {
            close(shm_fd_);
            throw std::runtime_error(std::format("ftruncate failed: {}", strerror(errno)));
        }

        queue_size_ = queue_size;
        buffer_capacity_ = queue_size_ - sizeof(QueueMeta);

        data_ = reinterpret_cast<char*>(map_shm(shm_fd_, queue_size_));
        meta_ = new (data_) QueueMeta{};
        meta_->version = kProtocolVersion;
        meta_->head.store(0);
        meta_->tail.store(0);
    }

    void WriteMessage(uint64_t position, uint32_t type, std::span<const std::byte> payload) {
        char* buffer = data_ + sizeof(QueueMeta);
        auto msg_size = sizeof(MessageHeader) + payload.size();

        MessageHeader header{type, payload.size()};

        size_t offset = position % buffer_capacity_;
        auto header_ptr = reinterpret_cast<const char*>(&header);

        if (offset + msg_size <= buffer_capacity_) {
            std::memcpy(buffer + offset, &header, sizeof(header));
            std::memcpy(buffer + offset + sizeof(header), payload.data(), payload.size());
            return;
        }

        auto first_chunk = buffer_capacity_ - offset;
        std::memcpy(buffer + offset, header_ptr, std::min(first_chunk, sizeof(header)));
        if (first_chunk < sizeof(header)) {
            std::memcpy(buffer, header_ptr + first_chunk, sizeof(header) - first_chunk);
            std::memcpy(buffer + sizeof(header) - first_chunk, payload.data(), payload.size());
        }
        else {
            auto payload_in_first = first_chunk - sizeof(header);
            std::memcpy(buffer + offset + sizeof(header), payload.data(), payload_in_first);
            std::memcpy(buffer, payload.data() + payload_in_first,
                        payload.size() - payload_in_first);
        }
    }

    const char* path_;
    size_t queue_size_;
    size_t buffer_capacity_;
    int shm_fd_ = -1;
    char* data_ = nullptr;
    QueueMeta* meta_ = nullptr;
};

struct ReceivedMessage {
    uint32_t type;
    std::span<std::byte> payload;
};

class ConsumerNode {
public:
    ConsumerNode(const char* path, size_t queue_size)
        : queue_size_(queue_size), buffer_capacity_(queue_size - sizeof(QueueMeta)) {
        shm_fd_ = shm_open(path, O_RDWR, 0666);
        if (shm_fd_ == -1) {
            throw std::runtime_error(std::format("shm_open failed: {}", strerror(errno)));
        }

        data_ = reinterpret_cast<char*>(map_shm(shm_fd_, queue_size_));
        meta_ = reinterpret_cast<QueueMeta*>(data_);

        try {
            validate_version(meta_);
        }
        catch (...) {
            munmap(data_, queue_size_);
            close(shm_fd_);
            data_ = nullptr;
            shm_fd_ = -1;
            throw;
        }
    }

    ~ConsumerNode() {
        if (data_) {
            munmap(data_, queue_size_);
        }
        if (shm_fd_ >= 0) {
            close(shm_fd_);
        }
    }

    ConsumerNode(const ConsumerNode&) = delete;
    ConsumerNode& operator=(const ConsumerNode&) = delete;

    std::optional<ReceivedMessage> Recv(uint32_t filter_type) {
        std::optional<ReceivedMessage> result;

        while (true) {
            auto head = meta_->head.load(std::memory_order_relaxed);
            auto tail = meta_->tail.load(std::memory_order_acquire);

            if (head >= tail) {
                return std::nullopt;
            }

            auto [header, msg_size] = ReadHeader(head);

            if (header.type == filter_type) {
                result.emplace();
                result->type = header.type;
                if (header.length > 0) {
                    buffer_storage_ = std::make_unique<std::byte[]>(header.length);
                    result->payload = {buffer_storage_.get(), header.length};
                    ReadPayload(head, header.length, buffer_storage_.get());
                }
                else {
                    result->payload = {};
                }
            }

            meta_->head.store(head + msg_size, std::memory_order_release);

            if (header.type == filter_type) {
                return result;
            }
        }
    }

private:
    std::pair<MessageHeader, size_t> ReadHeader(uint64_t position) {
        auto buffer = data_ + sizeof(QueueMeta);
        auto cap = buffer_capacity_;
        size_t offset = position % cap;

        MessageHeader header;
        if (offset + sizeof(MessageHeader) <= cap) {
            std::memcpy(&header, buffer + offset, sizeof(header));
        }
        else {
            std::memcpy(&header, buffer + offset, cap - offset);
            std::memcpy(reinterpret_cast<char*>(&header) + (cap - offset), buffer,
                        sizeof(header) - (cap - offset));
        }
        return {header, sizeof(MessageHeader) + header.length};
    }

    void ReadPayload(uint64_t position, size_t length, std::byte* dest) {
        if (length == 0) return;

        auto buffer = data_ + sizeof(QueueMeta);
        auto cap = buffer_capacity_;
        size_t offset = (position + sizeof(MessageHeader)) % cap;

        if (offset + length <= cap) {
            std::memcpy(dest, buffer + offset, length);
        }
        else {
            size_t first_part = cap - offset;
            std::memcpy(dest, buffer + offset, first_part);
            std::memcpy(dest + first_part, buffer, length - first_part);
        }
    }

    size_t queue_size_;
    size_t buffer_capacity_;
    int shm_fd_ = -1;
    char* data_ = nullptr;
    QueueMeta* meta_ = nullptr;
    std::unique_ptr<std::byte[]> buffer_storage_;
};

}  // namespace ipc
