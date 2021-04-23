#pragma once

#include <boost/asio.hpp>
#include <vector>
#include <mutex>

#include "Timer.hpp"

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

struct ClientsInfo
{
    std::size_t stopped_clients = 0;
    std::size_t total_clients = 0;
};

struct ClientContext
{
    boost::asio::io_context ctx;
    ProxyingInfo proxying_info;
    std::size_t testing_buffer_size = 0;
    ClientsInfo clients_info;
    std::mutex mutex;
    Timer timer;
};

class Client
{

public:

    explicit Client(ClientContext& client_ctx, int id);
    void Connect();

private:
    void StartHandshake();
    void CompleteHandshake();

    void StartProtocolPart();
    void CompleteProtocolPart();

    void WriteMessage();
    void ReadMessage();

    void StopTesting();

    ClientContext& my_ctx;
    boost::asio::ip::tcp::socket socket;
    Buffer send_buffer;
    Buffer recv_buffer;

    int my_id = 0;
};


