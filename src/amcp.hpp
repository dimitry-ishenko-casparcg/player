////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef AMCP_HPP
#define AMCP_HPP

#include <asio.hpp>
#include <string>

////////////////////////////////////////////////////////////////////////////////
namespace amcp
{

class connection
{
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket socket_;

    void async_send(const std::string&);

public:
    connection(asio::io_context&, const std::string& server);

    void osc_enable(asio::ip::port_type);
    void osc_disable(asio::ip::port_type);
};

}

////////////////////////////////////////////////////////////////////////////////
#endif
