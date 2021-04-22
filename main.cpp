#include "Client.hpp"

int main(int argc, char* argv[])
{
    ClientContext ctx;

    ctx.proxying_info.proxy.ip = "127.0.0.1";
    ctx.proxying_info.third_party.ip = "127.0.0.1";

    ctx.proxying_info.proxy.port = 5005;
    ctx.proxying_info.third_party.port = 6000;

    Client client(ctx);
    client.Connect();

    ctx.ctx.run();
    return 0;
}

