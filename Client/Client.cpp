#include "Client.hpp"
#include <iostream>
#include <boost/endian.hpp>

Client::Client(ClientContext &client_ctx)  : my_ctx{client_ctx}, socket{my_ctx.ctx}
{

}

void Client::Connect()
{
    boost::asio::ip::tcp::endpoint proxy_ep{boost::asio::ip::address::from_string(my_ctx.proxying_info.proxy.ip), my_ctx.proxying_info.proxy.port};
    socket.async_connect(proxy_ep, [this](const boost::system::error_code& error)
    {
        if(!error)
        {
            StartHandshake();
            std::cout << "Connected to proxy\n";
        }

        else std::cerr << "Error on connect to proxy " << error.message() << "\n";
    });
}

void Client::StartHandshake()
{
    send_buffer = {0x05, 0x01, 0x00};

    boost::asio::async_write(socket, boost::asio::buffer(send_buffer, send_buffer.size()), [this](const boost::system::error_code& error,std::size_t count)
    {
        if(!error)
        {
            CompleteHandshake();
            std::cout << "Handshake message was sent\n";
        }

        else std::cerr << "Handshake starting failed " << error.message() << "\n";
    });
}

void Client::CompleteHandshake()
{
    recv_buffer.resize(2);
    boost::asio::async_read(socket, boost::asio::buffer(recv_buffer, 2),[this](const boost::system::error_code& error,std::size_t count)
    {
        if(!error)
        {
            StartProtocolPart();
            std::cout << "Handshake message was received\n";
        }

        else std::cerr << "Handshake completion failed " << error.message() << "\n";
    });
}

void Client::StartProtocolPart()
{
    send_buffer = {0x05, 0x01, 0x00, 0x01};
    auto ip_bytes = boost::asio::ip::address::from_string(my_ctx.proxying_info.third_party.ip).to_v4().to_bytes();
    send_buffer.insert(send_buffer.end(), ip_bytes.begin(), ip_bytes.end());

    send_buffer.push_back(boost::endian::native_to_big(my_ctx.proxying_info.third_party.port) & 0xFF);
    send_buffer.push_back(boost::endian::native_to_big(my_ctx.proxying_info.third_party.port) >> 8);

    boost::asio::async_write(socket, boost::asio::buffer(send_buffer,send_buffer.size()),[this](const boost::system::error_code& error, std::size_t count)
    {
        if(!error)
        {
            CompleteProtocolPart();
            std::cout << "Protocol message was sent with " << count << " bytes \n";
        }

        else std::cerr << "Protocol part starting failed " << error.message() << "\n";
    });
}

void Client::CompleteProtocolPart()
{
    recv_buffer.resize(send_buffer.size());
    boost::asio::async_read(socket, boost::asio::buffer(recv_buffer,recv_buffer.size()),[this](const boost::system::error_code& error, std::size_t count)
    {
        if(!error)
        {
            send_buffer.resize(1024);
            recv_buffer.resize(send_buffer.size());
            WriteMessage();
            std::cout << "Protocol message was received\n";
        }

        else std::cerr << "Protocol part completion failed " << error.message() << "\n";
    });
}

void Client::WriteMessage()
{
    boost::asio::async_write(socket, boost::asio::buffer(&send_buffer[send_buf_pos],send_buffer.size() - send_buf_pos),[this](const boost::system::error_code& error,std::size_t count)
    {
        if(!error)
        {
            ReadMessage();
            std::cout << "Sent " << count << " bytes\n";
        }

        else std::cerr << "Error in message writing " << error.message() << "\n";
    });
}

void Client::ReadMessage()
{
    socket.async_receive(boost::asio::buffer(&recv_buffer[recv_buf_pos],recv_buffer.size()),[this](const boost::system::error_code& error,std::size_t count)
    {
        if(!error)
        {
            //recv_buf_pos += count;
            std::cout << "Received " << count << " bytes\n";
            WriteMessage();
        }

        else std::cerr << "Error in message reading " << error.message() << "\n";
    });
}
