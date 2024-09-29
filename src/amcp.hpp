////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#ifndef AMCP_HPP
#define AMCP_HPP

#include "types.hpp"

#include <asio.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
namespace amcp
{

class connection
{
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket socket_;

    void async_send(std::string cmd)
    {
        cmd += "\r\n";
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

    void osc_enable (int port) { async_send("OSC SUBSCRIBE "   + q(port)); }
    void osc_disable(int port) { async_send("OSC UNSUBSCRIBE " + q(port)); }

    void clear(int chan) { async_send("CLEAR " + q(chan)); }

    void mixer(int chan, int layer, const std::string& cmd, const std::vector<double>& args)
    {
        std::string sas;
        for (auto&& arg : args) sas += ' ' + q(arg);
        async_send("MIXER " + q(chan, layer) + ' ' + cmd + sas);
    }

    void loadbg(int chan, int layer, const std::string& path, std::optional<int> len = {})
    {
        std::string sol;
        if (len) sol = " LENGTH " + q(*len);
        async_send("LOADBG " + q(chan, layer) + ' ' + q(path) + sol + " MIX 30 AUTO FIT");
    }
};

}

////////////////////////////////////////////////////////////////////////////////
#endif
