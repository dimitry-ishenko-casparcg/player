////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "player.hpp"
#include "types.hpp"

#include <exception>
#include <filesystem>
#include <pgm/args.hpp>
#include <print>
#include <string>
#include <utility>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    auto name = fs::path{argv[0]}.filename().string();

    pgm::args args
    {
        { "-h", "--help",    "Show this help message and exit."   },
        { "-v", "--version", "Show program version and exit."     },

        { "path",            "Directory from which to play the media." },
    };

    std::exception_ptr ep;
    try { args.parse(argc, argv); }
    catch (...) { ep = std::current_exception(); }

    if (args["--help"])
    {
        std::print("{}\n", args.usage(name));
        return 0;
    }
    else if (args["--version"])
    {
        std::print("{} version {}\n", name, VERSION);
        return 0;
    }
    else if (ep) std::rethrow_exception(ep);

    ////////////////////
    settings data{ args["path"].value() };
    if (!fs::is_directory(data.media_path)) throw pgm::invalid_argument{
        "'" + data.media_path.string() + "' is not a directory"
    };

    player player{ std::move(data) };
    player.run();

    return 0;
}
catch (const std::exception& e)
{
    std::print(stderr, "{}\n", e.what());
    return 1;
}
