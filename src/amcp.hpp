////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef AMCP_HPP
#define AMCP_HPP

#include <asio.hpp>
#include <stdexcept>
#include <string>
#include <system_error>

////////////////////////////////////////////////////////////////////////////////
namespace amcp
{

class connection
{
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket socket_;

    void async_send(const std::string& cmd)
    {
        asio::async_write(socket_, asio::buffer(cmd),
            [](asio::error_code ec, std::size_t){ if (ec) throw std::system_error{ec}; }
        );
    }

public:
    connection(asio::io_context& io, const std::string& server) :
        resolver_{io}, socket_{io}
    {
        auto p = server.rfind(':');
        if (p == server.npos) throw std::invalid_argument{"Port number not specified"};

        auto host = server.substr(0, p), port = server.substr(p + 1);
        auto remote = resolver_.resolve(host, port);

        asio::connect(socket_, remote);
    }

    void osc_enable(asio::ip::port_type port)
    {
        async_send("OSC SUBSCRIBE " + std::to_string(port) + "\r\n");
    }
    void osc_disable(asio::ip::port_type port)
    {
        async_send("OSC UNSUBSCRIBE " + std::to_string(port) + "\r\n");
    }
};

}

////////////////////////////////////////////////////////////////////////////////
#endif
