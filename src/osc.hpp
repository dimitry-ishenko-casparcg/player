////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef OSC_HPP
#define OSC_HPP

#include <asio.hpp>
#include <functional>
#include <osc++.hpp>
#include <system_error>

////////////////////////////////////////////////////////////////////////////////
namespace osc
{

using recv_callback = std::function<void(const osc::packet&)>;

class connection
{
    asio::ip::udp::socket socket_;
    recv_callback callback_;;

    void async_recv()
    {
        socket_.async_wait(socket_.wait_read, [&](asio::error_code ec)
        {
            if (ec) throw std::system_error{ec};

            auto size = static_cast<osc::int32>(socket_.available());
            osc::packet packet{size};

            socket_.receive(asio::buffer(packet.data(), packet.size()));
            if (callback_) callback_(packet);

            async_recv();
        });
    }

public:
    explicit connection(asio::io_context& io) : socket_{io}
    {
        asio::ip::udp::endpoint local{asio::ip::address_v4::any(), 0};

        socket_.open(asio::ip::udp::v4());
        socket_.bind(local);

        async_recv();
    }

    auto port() const { return socket_.local_endpoint().port(); }

    void on_packet_recv(recv_callback cb) { callback_ = std::move(cb); }
};

}

////////////////////////////////////////////////////////////////////////////////
#endif
