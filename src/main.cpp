////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "player.hpp"
#include "settings.hpp"

#include <exception>
#include <filesystem>
#include <pgm/args.hpp>
#include <print>
#include <string>
#include <utility>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
void show_usage(const pgm::args& args, const std::string& name);
void show_version(const std::string& name);

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    auto name = fs::path{argv[0]}.filename().string();

    pgm::args args
    {
        { "-h", "--help",    "Show this help message and exit."   },
        { "-v", "--version", "Show program version and exit."     },

        { "path",            "Path from which to play the media." },
    };

    std::exception_ptr ep;
    try { args.parse(argc, argv); }
    catch (...) { ep = std::current_exception(); }

    if (args["--help"])
    {
        show_usage(args, name);
        return 0;
    }
    else if (args["--version"])
    {
        show_version(name);
        return 0;
    }
    else if (ep) std::rethrow_exception(ep);

    ////////////////////
    settings data{ args["path"].value() };

    player player{ std::move(data) };
    player.run();

    return 0;
}
catch (const std::exception& e)
{
    std::print(stderr, "{}\n", e.what());
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
void show_usage(const pgm::args& args, const std::string& name)
{
    std::print("{}\n", args.usage(name));
}

void show_version(const std::string& name)
{
    std::print("{} version {}\n", name, VERSION);
}
