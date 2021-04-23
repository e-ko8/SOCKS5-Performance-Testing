#include "Client.hpp"
#include <thread>

int main(int argc, char* argv[])
{
    int threads_size = 2;
    int clients_size = 5;
    int upped_bound_timer = 2;

    ClientContext ctx{.timer = Timer(upped_bound_timer)};

    ctx.proxying_info.proxy.ip = "127.0.0.1";
    ctx.proxying_info.third_party.ip = "127.0.0.1";

    ctx.proxying_info.proxy.port = 5007;
    ctx.proxying_info.third_party.port = 6000;

    ctx.testing_buffer_size = 1024;

    std::vector<Client> clients;
    clients.reserve(clients_size);

    for(int i = 0; i < clients_size; i++)
    {
        clients.emplace_back(Client{ctx,i});
    }

    for(auto& client : clients)
    {
        client.Connect();
    }

    std::vector<std::thread> threads;
    threads.reserve(threads_size);

    auto task = [&]()
    {
        ctx.ctx.run();
    };

    for(int i = 0; i < threads_size; i++)
    {
        threads.emplace_back(std::thread{task});
    }

    for(auto& th : threads)
    {
        th.join();
    }

    ctx.timer.PrintPerformanceInfo();

    return 0;
}

