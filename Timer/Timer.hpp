#pragma once
#include <chrono>
#include <map>
#include <iostream>

struct ClientPerformance
{
    std::chrono::time_point<std::chrono::high_resolution_clock> begin;
    std::chrono::time_point<std::chrono::high_resolution_clock> end;
    std::chrono::duration<double> elapsed = {};
    std::size_t total_messages_processed = 0;
};

class Timer
{
public:

    explicit Timer(double upped_bound)
    {
        std::cout << "Waiting for " << upped_bound << " seconds...\n\n";
        upped_bound_time = upped_bound;
    }

    void Start(int id)
    {
        clients_performance[id].begin = std::chrono::high_resolution_clock::now();
    }

    void Stop(int id)
    {
        clients_performance[id].end = std::chrono::high_resolution_clock::now();
        clients_performance[id].elapsed += std::chrono::duration<double>
                (std::chrono::duration_cast<std::chrono::nanoseconds>
                        (clients_performance[id].end-clients_performance[id].begin));
    }

    [[nodiscard]] double GetElapsed(int id) const
    {
        return clients_performance.at(id).elapsed.count();
    }

    void IncreaseProcessedMsgs(int id)
    {
        ++clients_performance[id].total_messages_processed;
    }

    [[nodiscard]] bool IsBoundReached(int id) const
    {
        return GetElapsed(id) >= upped_bound_time;
    }

    void PrintPerformanceInfo()
    {
        std::size_t total_messages_processed = 0;
        for(const auto& [id, client_perf] : clients_performance)
        {
            total_messages_processed += client_perf.total_messages_processed;
        }

        std::cout << "Total clients: " << clients_performance.size() << "\n";
        std::cout << "Elapsed time: " << upped_bound_time << " (seconds)\n";
        std::cout << "Total messages processed: " << total_messages_processed << "\n";
        std::size_t total_performance = total_messages_processed / static_cast<std::size_t>(upped_bound_time);
        std::cout << "Total performance: " << total_performance << " (msg/sec)\n";
        std::cout << "Performance per client: " << total_performance / clients_performance.size() << " (msg/sec)\n";
    }

private:

    std::map<int, ClientPerformance> clients_performance;
    double upped_bound_time = 0.0;
};


