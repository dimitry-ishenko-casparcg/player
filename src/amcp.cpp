////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "amcp.hpp"

#include <stdexcept>
#include <system_error>

////////////////////////////////////////////////////////////////////////////////
namespace amcp
{

connection::connection(asio::io_context& io, const std::string& server) :
    resolver_{io}, socket_{io}
{
    auto p = server.rfind(':');
    if (p == server.npos) throw std::invalid_argument{"Port number not specified"};

    auto host = server.substr(0, p), port = server.substr(p + 1);
    auto endpoint = resolver_.resolve(host, port);

    asio::error_code ec;
    asio::connect(socket_, endpoint, ec);
    if (ec) throw std::system_error{ec};
}

void connection::async_send(const std::string& cmd)
{
    asio::async_write(socket_, asio::buffer(cmd), [](asio::error_code ec, std::size_t)
    {
        if (ec) throw std::system_error{ec};
    });
}

void connection::osc_enable(asio::ip::port_type port)
{
    async_send("OSC SUBSCRIBE " + std::to_string(port) + "\r\n");
}

void connection::osc_disable(asio::ip::port_type port)
{
    async_send("OSC UNSUBSCRIBE " + std::to_string(port) + "\r\n");
}

}
