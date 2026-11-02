////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "player.hpp"
#include "types.hpp"

#include <charconv>
#include <cmath>
#include <exception>
#include <filesystem>
#include <pgm/args.hpp>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>
#include <utility>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
std::tuple<int, int> parse2(std::string_view val, char sep)
{
    auto p = val.find(sep);
    if (p == val.npos) pgm::invalid_argument{"missing separator"};

    int n1 = 0;
    auto [p1, ec1] = std::from_chars(val.data(), val.data() + p, n1);
    if (ec1 != std::errc{}) throw pgm::invalid_argument{"bad first value"};

    int n2 = 0;
    auto [p2, ec2] = std::from_chars(val.data() + p + 1, val.data() + val.size(), n2);
    if (ec2 != std::errc{}) throw pgm::invalid_argument{"bad second value"};

    return { n1, n2 };
}

constexpr auto eq(double x, double y) { return std::abs(x - y) < 1e-9; };

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    auto name = fs::path{argv[0]}.filename().string();

    pgm::args args
    {
        { "-s", "--size",       "WxH",  "Window size or screen resolution (in full-screen mode).\n"
                                        "Specified as <width>x<height>. Default: 1920x1080.\n" },

        { "-p", "--pos",        "X,Y",  "Window position. Specified as <x>,<y>. May not work\n"
                                        "on all desktops.\n" },

        { "-r", "--rate",       "N/D",  "Frame rate. Specified as a fraction <num>/<denom> (e.g.,\n"
                                        "60000/1001, 50000/1000. Default: 60000/1000.\n" },

        { "-c", "--cadence",    "C",    "Audio cadence. Specified as a repeating sequence of N\n"
                                        "sample counts whose sum equals 48000 × N ÷ frame rate.\n"
                                        "For common frame rates, the default cadence is:\n"
                                        "  23.976fps > 2002\n"
                                        "  24fps     > 2000\n"
                                        "  25fps     > 1920\n"
                                        "  29.97fps  > 1602,1601,1602,1601,1602\n"
                                        "  30fps     > 1600\n"
                                        "  50fps     >  960\n"
                                        "  59.94fps  >  801,801,800,801,801\n"
                                        "  60fps     >  800\n"
                                        "For others, it must be specified manually.\n" },

        { "-F", "--full-screen",        "Switch to full-screen mode.\n" },

        { "-b", "--border",             "Display window with border. If neither --full-screen nor\n"
                                        "--border is specified, a borderless window is displayed.\n" },

        { "-t", "--topmost",            "Display window above others.\n" },

        { "-m", "--mode",       "M",    "Image stretch mode. Must be one of: original, stretch,\n"
                                        "fit, fill, hfill, and vfill. Default: stretch.\n" },

        { "-h", "--help",               "Show this help message and exit."   },
        { "-v", "--version",            "Show program version and exit."     },

        { "path",                       "Directory from which to play the media." },
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

    // TODO: parse config files

    auto& size = args["--size"];
    if (size) std::tie(data.size.width, data.size.height) = parse2(size.value(), 'x');

    auto& pos = args["--pos"];
    if (pos) std::tie(data.pos.x, data.pos.y) = parse2(pos.value(), ',');

    auto& rate = args["--rate"];
    if (rate) std::tie(data.rate.num, data.rate.den) = parse2(rate.value(), '/');

    auto& cadence = args["--cadence"];
    if (cadence) data.cadence = cadence.value();
    else
    {
        auto fr = (double)data.rate.num / data.rate.den;
             if (eq(fr, 23.976)) data.cadence = "2002";
        else if (eq(fr, 24    )) data.cadence = "2000";
        else if (eq(fr, 25    )) data.cadence = "1920";
        else if (eq(fr, 29.97 )) data.cadence = "1602,1601,1602,1601,1602";
        else if (eq(fr, 30    )) data.cadence = "1600";
        else if (eq(fr, 50    )) data.cadence =  "960";
        else if (eq(fr, 59.94 )) data.cadence =  "801,801,800,801,801";
        else if (eq(fr, 60    )) data.cadence =  "800";
        else throw pgm::missing_argument{"missing cadence for non-standard rate"};
    }

    if (args["--full-screen"]) data.full_screen = true;
    if (args["--border"]) data.border = true;
    if (args["--topmost"]) data.topmost = true;

    auto& mode = args["--mode"];
    if (mode)
    {
        data.mode = mode.value();

        static constexpr auto modes = { "original", "stretch", "fit", "fill", "hfill", "vfill" };
        if (!std::ranges::contains(modes, data.mode)) throw std::invalid_argument{"bad mode"};
    }

    ////////////////////
    player player{ std::move(data) };
    player.run();

    return 0;
}
catch (const std::exception& e)
{
    std::print(stderr, "{}\n", e.what());
    return 1;
}
