#include "queue.h"

#include <iostream>
#include <string_view>
#include <thread>

int main() {
    static constexpr const char* kQueuePath = "/ipc_queue__";
    static constexpr size_t kQueueSize = 1024;
    static constexpr uint32_t kMessageType = 1;

    ipc::ConsumerNode consumer(kQueuePath, kQueueSize);

    std::cout << "[CONSUMER] listening for messages (type=" << kMessageType << ")" << std::endl;

    while (true) {
        auto msg = consumer.Recv(kMessageType);
        if (!msg.has_value()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        std::string_view text(reinterpret_cast<const char*>(msg->payload.data()), msg->payload.size());
        std::cout << "[CONSUMER] received " << msg->payload.size() << " bytes: " << text << std::endl;
    }
}
