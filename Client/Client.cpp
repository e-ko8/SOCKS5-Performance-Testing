#include "Client.hpp"
#include <iostream>
#include <boost/endian.hpp>

Client::Client(ClientContext &client_ctx, int id)  : my_ctx{client_ctx}, socket{my_ctx.ctx}
{
    my_id = id;
}

void Client::Connect()
{
    boost::asio::ip::tcp::endpoint proxy_ep{boost::asio::ip::address::from_string(my_ctx.proxying_info.proxy.ip), my_ctx.proxying_info.proxy.port};
    socket.async_connect(proxy_ep, [this](const boost::system::error_code& error)
    {
        if(!error)
        {
            StartHandshake();
        }

        else
        {
            std::cerr << "Error on connect to proxy by id " << my_id << ":" << error.message() << "\n";
            StopTesting();
        }
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
        }

        else
        {
            std::cerr << "Handshake starting failed by id " << my_id << ":" << error.message() << "\n";
            StopTesting();
        }
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
        }

        else
        {
            std::cerr << "Handshake completion failed by id " << my_id << ":"  << error.message() << "\n";
            StopTesting();
        }
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
        }

        else
        {
            std::cerr << "Protocol part starting failed by id " << my_id << ":"  << error.message() << "\n";
            StopTesting();
        }
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
        }

        else
        {
            std::cerr << "Protocol part completion failed by id " << my_id << ":"  << error.message() << "\n";
            StopTesting();
        }
    });
}

void Client::WriteMessage()
{
    {
        std::lock_guard lock(my_ctx.mutex);
        my_ctx.timer.Start(my_id);
    }

    boost::asio::async_write(socket, boost::asio::buffer(&send_buffer.buf[send_buffer.pos],send_buffer.buf.size() - send_buffer.pos),[this](const boost::system::error_code& error,std::size_t count)
    {
        if(!error)
        {
            send_buffer.pos += count;
            ReadMessage();
        }

        else
        {
            std::cerr << "Error in message writing by id " << my_id << ":"  << error.message() << "\n";
            StopTesting();
        }
    });
}

void Client::ReadMessage()
{
    socket.async_receive(boost::asio::buffer(&recv_buffer.buf[recv_buffer.pos],recv_buffer.buf.size() - recv_buffer.pos),[this](const boost::system::error_code& error,std::size_t count)
    {
        if(!error)
        {
            recv_buffer.pos += count;

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

            {
                {
                    std::lock_guard lock(my_ctx.mutex);
                    my_ctx.timer.Stop(my_id);
                    my_ctx.timer.IncreaseProcessedMsgs(my_id);
                }

                if(my_ctx.timer.IsBoundReached(my_id))
                {
                    StopTesting();
                    return;
                }
            }

            WriteMessage();
        }

        else
        {
            std::cerr << "Error in message reading by id " << my_id << ":"  << error.message() << "\n";
            StopTesting();
        }
    });
}

void Client::StopTesting()
{
    std::lock_guard lock(my_ctx.mutex);
    my_ctx.clients_info.stopped_clients++;

    if(my_ctx.clients_info.stopped_clients == my_ctx.clients_info.total_clients)
    {
        my_ctx.ctx.stop();
    }
}
