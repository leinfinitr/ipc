#include <algorithm>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "ipc/ipc.h"

// Configuration
const int NUM_ITERATIONS = 1000;
const int WARMUP_ITERATIONS = 100;
const int MSG_SIZE = 64; // Message size in bytes

int64_t current_timestamp()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

int main()
{
    // Initialize separate channels for sending and receiving
#ifdef _WIN32
    ipc::node sender("ipc-latency-request", ipc::LinkType::Sender, ipc::ChannelType::NamedPipe);
    ipc::node receiver("ipc-latency-response", ipc::LinkType::Receiver, ipc::ChannelType::NamedPipe);
#else
    ipc::node sender("ipc-latency-request");
    ipc::node receiver("ipc-latency-response");
#endif

    std::cout << "Connecting to IPC server..." << std::endl;
    std::cout << "Sending on channel: ipc-latency-request" << std::endl;
    std::cout << "Receiving on channel: ipc-latency-response" << std::endl;

    // Create a message buffer with timestamp
    std::vector<char> buffer(MSG_SIZE);

    // Warmup
    std::cout << "Warming up for " << WARMUP_ITERATIONS << " iterations..." << std::endl;
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        // Create message with current timestamp
        int64_t send_time = current_timestamp();
        std::memcpy(buffer.data(), &send_time, sizeof(int64_t));

        // Send message
        sender.send(buffer.data(), buffer.size());

        // Wait for reply
        auto reply = receiver.receive();
        if (!reply) {
            std::cerr << "Error receiving reply during warmup" << std::endl;
            continue;
        }
    }

    // Actual latency test
    std::vector<double> latencies;
    latencies.reserve(NUM_ITERATIONS);

    std::cout << "Running latency test for " << NUM_ITERATIONS << " iterations..." << std::endl;
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Create message with current timestamp
        std::this_thread::sleep_for(std::chrono::microseconds(1000));
        int64_t send_time = current_timestamp();
        std::memcpy(buffer.data(), &send_time, sizeof(int64_t));

        // Send message
        sender.send(buffer.data(), buffer.size());

        // Wait for reply
        auto reply = receiver.receive();
        if (!reply) {
            std::cerr << "Error receiving reply" << std::endl;
            continue;
        }

        // Calculate round trip time
        int64_t recv_time = current_timestamp();
        double latency = static_cast<double>(recv_time - send_time);
        latencies.push_back(latency);
    }

    // Calculate statistics
    double min_latency = *std::min_element(latencies.begin(), latencies.end());
    double max_latency = *std::max_element(latencies.begin(), latencies.end());
    double avg_latency = std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();

    // Calculate median and percentiles
    std::sort(latencies.begin(), latencies.end());
    double median = latencies[latencies.size() / 2];
    double p95 = latencies[static_cast<size_t>(latencies.size() * 0.95)];
    double p99 = latencies[static_cast<size_t>(latencies.size() * 0.99)];

    // Print results
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\nLatency Results (IPC channel, nanoseconds):" << std::endl;
    std::cout << "  Minimum: " << min_latency << " ns" << std::endl;
    std::cout << "  Maximum: " << max_latency << " ns" << std::endl;
    std::cout << "  Average: " << avg_latency << " ns" << std::endl;
    std::cout << "  Median:  " << median << " ns" << std::endl;
    std::cout << "  95th percentile: " << p95 << " ns" << std::endl;
    std::cout << "  99th percentile: " << p99 << " ns" << std::endl;

    return 0;
}
