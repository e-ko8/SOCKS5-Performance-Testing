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
    send_buffer.buf = {0x05, 0x01, 0x00};

    boost::asio::async_write(socket, boost::asio::buffer(send_buffer.buf, send_buffer.buf.size()), [this](const boost::system::error_code& error,std::size_t count)
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
    recv_buffer.buf.resize(2);
    boost::asio::async_read(socket, boost::asio::buffer(recv_buffer.buf, 2),[this](const boost::system::error_code& error,std::size_t count)
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
    send_buffer.buf = {0x05, 0x01, 0x00, 0x01};
    auto ip_bytes = boost::asio::ip::address::from_string(my_ctx.proxying_info.third_party.ip).to_v4().to_bytes();
    send_buffer.buf.insert(send_buffer.buf.end(), ip_bytes.begin(), ip_bytes.end());

    send_buffer.buf.push_back(boost::endian::native_to_big(my_ctx.proxying_info.third_party.port) & 0xFF);
    send_buffer.buf.push_back(boost::endian::native_to_big(my_ctx.proxying_info.third_party.port) >> 8);

    boost::asio::async_write(socket, boost::asio::buffer(send_buffer.buf,send_buffer.buf.size()),[this](const boost::system::error_code& error, std::size_t count)
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
    recv_buffer.buf.resize(send_buffer.buf.size());
    boost::asio::async_read(socket, boost::asio::buffer(recv_buffer.buf,recv_buffer.buf.size()),[this](const boost::system::error_code& error, std::size_t count)
    {
        if(!error)
        {
            send_buffer.buf.resize(my_ctx.testing_buffer_size);
            recv_buffer.buf.resize(send_buffer.buf.size());
            WriteMessage();
            std::cout << "Protocol message was received\n";
        }

        else std::cerr << "Protocol part completion failed " << error.message() << "\n";
    });
}

void Client::WriteMessage()
{
    boost::asio::async_write(socket, boost::asio::buffer(&send_buffer.buf[send_buffer.pos],send_buffer.buf.size() - send_buffer.pos),[this](const boost::system::error_code& error,std::size_t count)
    {
        if(!error)
        {
            std::cout << "Sent " << count << " bytes\n";
            send_buffer.pos += count;
            ReadMessage();
        }

        else std::cerr << "Error in message writing " << error.message() << "\n";
    });
}

void Client::ReadMessage()
{
    socket.async_receive(boost::asio::buffer(&recv_buffer.buf[recv_buffer.pos],recv_buffer.buf.size() - recv_buffer.pos),[this](const boost::system::error_code& error,std::size_t count)
    {
        if(!error)
        {
            std::cout << "Received " << count << " bytes\n";

            recv_buffer.pos += count;
            std::cout << "Recv buf pos = " << recv_buffer.pos << "\n";

            if(recv_buffer.pos != send_buffer.pos)
            {
                ReadMessage();
                return;
            }

            if(send_buffer.pos == send_buffer.buf.size())
            {
                send_buffer.pos = 0;
                recv_buffer.pos = 0;
            }

            std::cout << "\n";
            WriteMessage();
        }

        else std::cerr << "Error in message reading " << error.message() << "\n";
    });
}
