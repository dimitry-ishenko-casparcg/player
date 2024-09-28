////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2024 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "amcp.hpp"
#include "osc.hpp"

#include <asio.hpp>
#include <exception>
#include <filesystem>
#include <iostream>
#include <pgm/args.hpp>
#include <vector>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
struct settings
{
    std::string server = "localhost:5250";
    std::vector<fs::path> files;
};

settings read_setting();

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
    auto sett = read_setting();

    std::cout << "Connecting to " << sett.server << std::endl;
    amcp::connection control{io, sett.server};

    std::cout << "Subscribing to OSC" << std::endl;
    osc::connection monitor{io};
    control.osc_enable(monitor.port());
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
