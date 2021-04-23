#include "Client.hpp"

int main(int argc, char* argv[])
{
    ClientContext ctx;

    ctx.proxying_info.proxy.ip = "127.0.0.1";
    ctx.proxying_info.third_party.ip = "127.0.0.1";

    ctx.proxying_info.proxy.port = 5007;
    ctx.proxying_info.third_party.port = 6000;

    ctx.testing_buffer_size = 1024;

    Timer timer(5);
    Client client(ctx,0, timer);
    client.Connect();

    ctx.ctx.run();

    timer.PrintPerformanceInfo();
    return 0;
}

