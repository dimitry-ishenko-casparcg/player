////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "amcp.hpp"
#include "osc.hpp"

#include <asio.hpp>
#include <deque>
#include <exception>
#include <filesystem>
#include <iostream>
#include <osc++.hpp>
#include <pgm/args.hpp>
#include <vector>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
struct settings
{
    std::string server = "localhost:5250";
    int channel = 1;
    int layer = 0;

    std::vector<fs::path> files;
};

settings read_setting();

////////////////////////////////////////////////////////////////////////////////
class player
{
    amcp::connection control_;
    osc::connection monitor_;
    std::deque<std::string> paths_;

    osc::address_space space_;

    auto get_control(asio::io_context& io, const std::string& server)
    {
        std::cout << "Connecting to " << server << std::endl;
        return amcp::connection{io, server};
    }

    auto get_monitor(asio::io_context& io)
    {
        std::cout << "Setting up OSC connection" << std::endl;
        return osc::connection{io};
    }

    void load_next()
    {
        //
    }

public:
    player(asio::io_context& io, const settings& settings) :
        control_{get_control(io, settings.server)}, monitor_{get_monitor(io)}
    {
        auto address = "/channel/" + std::to_string(settings.channel) + "/stage/layer/" + std::to_string(settings.layer) + "/foreground/file/path";
        space_.add(address, [&](osc::message msg)
        {
            if (msg.values().are<std::string>())
            {
                std::string path;
                msg >> path;
                if (path == paths_.front()) load_next();
            }
        });

        monitor_.on_packet_recv([&](osc::packet packet)
        {
            try { space_.dispatch(packet.parse()); }
            catch(...) { } // discard invalid packets
        });

        std::cout << "Subscribing to OSC" << std::endl;
        control_.osc_enable(monitor_.port());
    }
};

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    pgm::args args
    {
        { "-v", "--version", "Show version and exit." },
        { "-h", "--help",    "Show this help screen and exit." },
    };
    args.parse(argc, argv);

    if(args["--version"])
    {
        std::cout << argv[0] << " " << VERSION << std::endl;
        return 0;
    }
    if(args["--help"])
    {
        std::cout << args.usage(argv[0]) << std::endl;
        return 0;
    }

    asio::io_context io;

    std::cout << "Reading settings" << std::endl;
    auto settings = read_setting();

    std::cout << "Creating player" << std::endl;
    player player{io, settings};
    //

    io.run();
    return 0;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
settings read_setting()
{
    // TODO: read settings from settings.ini
    return settings{};
}
