////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef OSC_HPP
#define OSC_HPP

#include <asio.hpp>

////////////////////////////////////////////////////////////////////////////////
namespace osc
{

class connection
{
    asio::ip::udp::socket socket_;

public:
    explicit connection(asio::io_context& io) : socket_{io}
    {
        asio::ip::udp::endpoint local{asio::ip::address_v4::any(), 0};

        socket_.open(asio::ip::udp::v4());
        socket_.bind(local);
    }

    auto port() const { return socket_.local_endpoint().port(); }
};

}

////////////////////////////////////////////////////////////////////////////////
#endif
