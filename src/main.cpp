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
    int chan = 1, layer = 0;

    std::deque<std::string> paths;
};

class player
{
    amcp::connection control_;
    osc::connection monitor_;
    settings sett_;

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

    void load_next()
    {
        if (sett_.paths.size())
        {
            std::cout << "Loading " << sett_.paths.front() << std::endl;
            control_.loadbg(sett_.chan, sett_.layer, sett_.paths.front());

            sett_.paths.emplace_back( std::move(sett_.paths.front()) );
            sett_.paths.pop_front();
        }
        else ready_ = false;
    }

public:
    player(asio::io_context& io, settings sett) :
        control_{get_control(io, sett.server)}, monitor_{get_monitor(io)}, sett_{std::move(sett)}
    {
        auto address = "/channel/" + q(sett_.chan) + "/stage/layer/" + q(sett_.layer) + "/background/producer";
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
        control_.clear(sett_.chan);
        control_.mixer(sett_.chan, sett_.layer, "ANCHOR", { .5, .5 });
        control_.mixer(sett_.chan, sett_.layer, "FILL", { .5, .5, 1, 1 });

        std::cout << "Subscribing to OSC" << std::endl;
        control_.osc_subscribe(monitor_.port());

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

    settings settings;

    asio::io_context io;
    player player{io, std::move(settings)};

    io.run();
    return 0;
}
catch(const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return 1;
}
