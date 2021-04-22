#pragma once

#include <boost/asio.hpp>
#include <vector>

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
    std::vector<std::uint8_t> send_buffer;
    std::vector<std::uint8_t> recv_buffer;
    std::size_t send_buf_pos = 0;
    std::size_t recv_buf_pos = 0;
};


