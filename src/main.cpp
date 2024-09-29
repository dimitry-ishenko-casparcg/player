////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "amcp.hpp"
#include "osc.hpp"
#include "types.hpp"

#include <asio.hpp>
#include <deque>
#include <exception>
#include <iostream>
#include <osc++.hpp>
#include <pgm/args.hpp>
#include <string>

////////////////////////////////////////////////////////////////////////////////
struct settings
{
    std::string server = "localhost:5250";
    int channel = 1;
    int layer = 0;

    std::deque<std::string> paths;
};

settings read_setting();

////////////////////////////////////////////////////////////////////////////////
class player
{
    amcp::connection control_;
    osc::connection monitor_;
    settings settings_;

    osc::address_space space_;
    bool ready_ = true;

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

    void reset()
    {
        control_.clear(settings_.channel);
        control_.mixer(settings_.channel, settings_.layer, "ANCHOR", { .5, .5 });
        control_.mixer(settings_.channel, settings_.layer, "FILL", { .5, .5, 1, 1 });
    }

    void load_next()
    {
        if (settings_.paths.size())
        {
            std::cout << "Loading " << settings_.paths.front() << std::endl;
            control_.loadbg(settings_.channel, settings_.layer, settings_.paths.front());

            settings_.paths.emplace_back( std::move(settings_.paths.front()) );
            settings_.paths.pop_front();
        }
        else ready_ = false;
    }

public:
    player(asio::io_context& io, settings settings) :
        control_{get_control(io, settings.server)}, monitor_{get_monitor(io)}, settings_{std::move(settings)}
    {
        auto address = "/channel/" + q(settings_.channel) + "/stage/layer/" + q(settings_.layer) + "/background/producer";
        space_.add(address, [&](osc::message msg)
        {
            if (msg.value(0).to_string() == "empty")
            {
                if (ready_)
                {
                    ready_ = false;
                    load_next();
                }
            }
            else ready_ = true;
        });

        monitor_.on_packet_recv([&](osc::packet packet)
        {
            try { space_.dispatch(packet.parse()); }
            catch(...) { } // discard invalid packets
        });

        std::cout << "Resetting channel" << std::endl;
        reset();

        std::cout << "Subscribing to OSC" << std::endl;
        control_.osc_enable(monitor_.port());

        std::cout << "Starting playback" << std::endl;
        load_next();
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

    player player{io, std::move(settings)};

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
