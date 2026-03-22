#include "queue.h"

#include <iostream>
#include <string>

int main() {
    static constexpr const char* kQueuePath = "/ipc_queue__";
    static constexpr size_t kQueueSize = 1024;
    static constexpr uint32_t kMessageType = 1;

    ipc::ProducerNode producer(kQueuePath, kQueueSize);

    std::string line;
    while (std::getline(std::cin, line)) {
        auto payload = std::as_bytes(std::span{line});
        if (producer.Send(kMessageType, payload)) {
            std::cout << "[PRODUCER] sent " << line.size() << " bytes" << std::endl;
        }
        else {
            std::cerr << "[PRODUCER] queue full, failed to send" << std::endl;
        }
    }

    std::cout << "[PRODUCER] exited" << std::endl;
    return 0;
}
