////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "player.hpp"
#include "types.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib> // std::getenv
#include <exception>
#include <filesystem>
#include <generator>
#include <initializer_list>
#include <pgm/args.hpp>
#include <print>
#include <ranges>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

////////////////////////////////////////////////////////////////////////////////
pgm::expected< std::vector<std::string_view> >
parse(std::string_view s, char ch, std::size_t min, std::size_t max)
{
    auto parts = std::views::split(s, ch)
        | std::views::transform([](auto p){ return std::string_view{p}; })
        | std::ranges::to<std::vector>();
    if (parts.size() < min || parts.size() > max)
        return pgm::unexpected("bad value '" + std::string{s} + "'");

    return parts;
}

template<typename V>
pgm::expected<V> parse_2(std::string_view s, char ch)
{
    [[maybe_unused]] auto [x, y] = V{ };
    return parse(s, ch, 2, 2).and_then([](auto&& p){
        return pgm::convert<decltype (x)>(p[0]).and_then([&](auto x){
            return pgm::convert<decltype (y)>(p[1]).transform([&](auto y){ return V{x, y}; });
        });
    });
}

template<typename V>
pgm::expected<V> parse_list(std::string_view s, const auto& il)
{
    if (!std::ranges::contains(il, s))
        return pgm::unexpected("bad value '" + std::string{s} + "'");
    return V{s};
}

template<> pgm::expected<size> pgm::convert(std::string_view s) { return parse_2<size>(s, 'x'); }
template<> pgm::expected<pos > pgm::convert(std::string_view s) { return parse_2<pos >(s, ','); }
template<> pgm::expected<rate> pgm::convert(std::string_view s) { return parse_2<rate>(s, '/'); }

template<> pgm::expected<msec> pgm::convert(std::string_view s) {
    return pgm::convert<unsigned>(s).transform([](auto v){ return msec{v}; });
}

struct mode { std::string_view val; };
template<> pgm::expected<mode> pgm::convert(std::string_view s) { return parse_list<mode>(s, modes); }

template<> pgm::expected<tran> pgm::convert(std::string_view s)
{
    auto val = parse(s, ',', 2, 4);
    if (!val) return pgm::unexpected(val.error());

    struct tran tran;
    auto& p = *val;

    struct type  { std::string_view val; };
    struct tween { std::string_view val; };
    struct dir   { std::string_view val; };

    if (auto v = parse_list<type>(p[0], types))
        tran.type = v->val;
    else return pgm::unexpected("bad type '" + std::string{p[0]} + "'");

    if (auto v = pgm::convert<unsigned>(p[1]))
        tran.len = msec{*v};
    else return pgm::unexpected("bad len '" + std::string{p[1]} + "'");

    for (std::size_t i = 2; i < p.size(); ++i)
    {
        if (auto v = parse_list<tween>(p[i], tweens))
        {
            if (tran.tween) return pgm::unexpected("bad dir '" + std::string{p[i]} + "'");
            tran.tween = v->val;
        }
        else if (auto v = parse_list<dir>(p[i], dirs))
        {
            if (tran.dir) return pgm::unexpected("bad tween '" + std::string{p[i]} + "'");
            tran.dir = v->val;
        }
        else return pgm::unexpected("bad tween/dir '" + std::string{p[i]} + "'");
    }

    return tran;
}

////////////////////////////////////////////////////////////////////////////////
constexpr auto eq(double x, double y) { return std::abs(x - y) < .001; };

std::generator<fs::path> conf_dirs()
{
    auto env = [](auto name) {
        auto val = std::getenv(name); return (val && *val) ? val : nullptr;
    };

#ifdef _WIN32
    if (auto val = env("APPDATA")) co_yield val;
    if (auto val = env("ProgramData")) co_yield val;
#else
    if (auto val = env("XDG_CONFIG_HOME")) co_yield val;
    else if (auto val = env("HOME")) co_yield fs::path{val} / ".config";

    const char* dirs = env("XDG_CONFIG_DIRS");
    if (!dirs) dirs = "/etc/xdg";

    for (auto val : std::string_view{dirs} | std::views::split(':'))
    {
        std::string_view dir{val};
        if (dir.size()) co_yield dir;
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
try
{
    auto name = fs::path{argv[0]}.filename().string();

    pgm::args args
    {
        { "-s", "--size", "S",      "Window size or screen resolution (in full-screen mode).\n"
                                    "Specified as <width>x<height>. Default: 1920x1080.\n" },

        { "-p", "--pos", "P",       "Window position. Specified as <x>,<y>. May not work\n"
                                    "on all desktops.\n" },

        { "-r", "--rate", "R",      "Frame rate. Specified as a fraction <num>/<denom>\n"
                                    "(e.g., 60000/1001, 50000/1000). Default: 60000/1000.\n" },

        { "-c", "--cadence", "C",   "Audio cadence. Specified as a repeating sequence of N\n"
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

        { "-F", "--full-screen",    "Switch to full-screen mode. May not work on all\n"
                                    "desktops.\n" },

        { "-b", "--border",         "Display window with border. If neither --full-screen\n"
                                    "nor --border is specified, a borderless window is.\n"
                                    "displayed.\n" },

        { "-t", "--topmost",        "Position window on top of others.\n" },

        { "-i", "--image-len", "L", "Display length for images in milliseconds.\n"
                                    "Default: 10000.\n" },

        { "-n", "--tran", "N",      "Transition. Specified as <type>,<len>[,tween][,dir].\n"
                                    "Type is be one of: cut, mix, push, slide, wipe,\n"
                                    "fadecut, cutfade, or vfade.\n"
                                    "Length is a number of milliseconds.\n"
                                    "Tween is either linear or ease<X><Y>, with X being:\n"
                                    "in, out, inout, or outin; and Y being: quad, cubic,\n"
                                    "quart, quint, sine, expo, circ, elastic, back,\n"
                                    "or bounce (e.g., easeinoutexpo).\n"
                                    "And, direction - which only applies to push, slide,\n"
                                    "and wipe - is one of: left (or fromright), right\n"
                                    "(or fromleft), up (or fromdown), or down (fromtop).\n" },

        { "-m", "--mode", "M",      "Image stretch mode. Must be one of: original, stretch,\n"
                                    "fit, fill, hfill, or vfill.\n"  },

        { "-h", "--help",           "Show this help message and exit." },
        { "-v", "--version",        "Show program version and exit."   },

        { "path",                   "Directory from which to play the media." },
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
    args.reset();

    for (auto&& dir : conf_dirs())
    {
        auto conf_file = dir / name / "player.conf";
        if (fs::is_regular_file(conf_file))
        {
            std::print("[-main-] reading '{}'\n", conf_file.string());
            args.parse(conf_file);
            break;
        }
    }

    auto conf_file = data.media_path / "player.conf";
    if (fs::is_regular_file(conf_file))
    {
        std::print("[-main-] reading '{}'\n", conf_file.string());
        args.parse(conf_file);
    }

    args.parse(argc, argv);

    if (auto& arg = args["--size"]) data.size = arg.value<size>();
    if (auto& arg = args["--pos" ]) data.pos  = arg.value<pos >();
    if (auto& arg = args["--rate"]) data.rate = arg.value<rate>();

    if (auto& arg = args["--cadence"]) data.cadence = arg.value();
    else
    {
        auto rate = (double)data.rate.num / data.rate.den;
        std::print("rate={}\n", rate);
             if (eq(rate, 23.976)) data.cadence = "2002";
        else if (eq(rate, 24    )) data.cadence = "2000";
        else if (eq(rate, 25    )) data.cadence = "1920";
        else if (eq(rate, 29.97 )) data.cadence = "1602,1601,1602,1601,1602";
        else if (eq(rate, 30    )) data.cadence = "1600";
        else if (eq(rate, 50    )) data.cadence =  "960";
        else if (eq(rate, 59.94 )) data.cadence =  "801,801,800,801,801";
        else if (eq(rate, 60    )) data.cadence =  "800";
        else throw pgm::missing_argument{"need cadence for non-standard frame rate"};
    }

    if (args["--full-screen"]) data.full_screen = true;
    if (args["--border"]) data.border = true;
    if (args["--topmost"]) data.topmost = true;

    if (auto& arg = args["--image-len"]) data.image_len = arg.value<msec>();

    if (auto& arg = args["--tran"]) data.tran = arg.value<tran>();
    if (auto& arg = args["--mode"]) data.mode = arg.value<mode>().val;

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
