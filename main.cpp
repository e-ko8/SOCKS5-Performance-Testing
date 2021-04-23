#include "Client.hpp"
#include "Starter.hpp"
#include <thread>

struct TestingCtx
{
    std::size_t threads_size = 0;
    int clients_size = 0;
    double upped_bound_timer = 0;
};

void StartPerformanceTesting(ClientContext& ctx, TestingCtx& testing_ctx);

int main(int argc, char* argv[])
{
    Starter starter("SOCKS5-Performance-Testing");
    starter.AddArgument("help", "Arguments description");
    starter.AddArgument("p_ip","Proxy ip", std::string{"127.0.0.1"});
    starter.AddArgument("tp_ip","Third party ip", std::string{"127.0.0.1"});

    starter.AddRequiredArgument<std::uint16_t>("p_port","Proxy port");
    starter.AddRequiredArgument<std::uint16_t>("tp_port","Third party port");
    starter.AddRequiredArgument<std::size_t>("threads","Threads number");
    starter.AddRequiredArgument<int>("clients","Clients number");
    starter.AddRequiredArgument<double>("limit","Time limit for testing (seconds)");
    starter.AddRequiredArgument<std::size_t>("bsize","Testing buffer size");

    starter.ParseArguments(argc,argv);

    if(!starter.Failed())
    {
        TestingCtx testing_ctx;

        starter.GetArgValue("threads", testing_ctx.threads_size);
        starter.GetArgValue("clients", testing_ctx.clients_size);
        starter.GetArgValue("limit", testing_ctx.upped_bound_timer);

        ClientContext ctx{.timer = Timer(testing_ctx.upped_bound_timer)};

        starter.GetArgValue("p_ip", ctx.proxying_info.proxy.ip);
        starter.GetArgValue("tp_ip", ctx.proxying_info.third_party.ip);

        starter.GetArgValue("p_port", ctx.proxying_info.proxy.port);
        starter.GetArgValue("tp_port", ctx.proxying_info.third_party.port);

        starter.GetArgValue("bsize", ctx.testing_buffer_size);

        StartPerformanceTesting(ctx,testing_ctx);

    }

    return 0;
}

void StartPerformanceTesting(ClientContext& ctx, TestingCtx& testing_ctx)
{
    std::vector<Client> clients;
    clients.reserve(testing_ctx.clients_size);

    ctx.clients_info.total_clients = testing_ctx.clients_size;

    for(int i = 0; i < testing_ctx.clients_size; i++)
    {
        clients.emplace_back(Client{ctx,i});
    }

    for(auto& client : clients)
    {
        client.Connect();
    }

    boost::asio::signal_set sigs{ctx.ctx,SIGINT,SIGTSTP, SIGTERM};
    sigs.async_wait([&](boost::system::error_code error,int signal_number)
    {
        if(!error)
        {
            ctx.ctx.stop();
        }
    });

    std::vector<std::thread> threads;
    threads.reserve(testing_ctx.threads_size);

    auto task = [&]()
    {
        ctx.ctx.run();
    };

    std::cout << "Running in " << testing_ctx.threads_size << " threads\n";

    for(int i = 0; i < testing_ctx.threads_size; i++)
    {
        threads.emplace_back(std::thread{task});
    }

    for(auto& thread : threads)
    {
        thread.join();
    }

    ctx.timer.PrintPerformanceInfo();
}

