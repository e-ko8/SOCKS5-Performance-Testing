#pragma once

#include <boost/asio.hpp>
#include <vector>

struct Buffer
{
    std::vector<std::uint8_t> buf;
    std::size_t pos = 0;
};

struct ServerInfo
{
    std::string ip;
    std::uint16_t port;
};

struct ProxyingInfo
{
    ServerInfo proxy;
    ServerInfo third_party;
};

struct ClientContext
{
    boost::asio::io_context ctx;
    ProxyingInfo proxying_info;
    std::size_t testing_buffer_size = 0;
};

class Client
{

public:

    explicit Client(ClientContext& client_ctx);
    void Connect();

private:
    void StartHandshake();
    void CompleteHandshake();

    void StartProtocolPart();
    void CompleteProtocolPart();

    void WriteMessage();
    void ReadMessage();

    ClientContext& my_ctx;
    boost::asio::ip::tcp::socket socket;
    Buffer send_buffer;
    Buffer recv_buffer;
};


