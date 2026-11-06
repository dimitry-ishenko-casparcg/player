////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <chrono>
#include <filesystem>
#include <initializer_list>
#include <optional>

namespace fs = std::filesystem;
using msec = std::chrono::milliseconds;

using namespace std::chrono_literals;

////////////////////////////////////////////////////////////////////////////////
constexpr auto types = { "cut", "mix", "push", "slide", "wipe", "fadecut", "cutfade", "vfade" };

constexpr auto tweens = { "linear",
    "easeinquad"   , "easeoutquad"   , "easeinoutquad"   , "easeoutinquad"   ,
    "easeincubic"  , "easeoutcubic"  , "easeinoutcubic"  , "easeoutincubic"  ,
    "easeinquart"  , "easeoutquart"  , "easeinoutquart"  , "easeoutinquart"  ,
    "easeinquint"  , "easeoutquint"  , "easeinoutquint"  , "easeoutinquint"  ,
    "easeinsine"   , "easeoutsine"   , "easeinoutsine"   , "easeoutinsine"   ,
    "easeinexpo"   , "easeoutexpo"   , "easeinoutexpo"   , "easeoutinexpo"   ,
    "easeincirc"   , "easeoutcirc"   , "easeinoutcirc"   , "easeoutincirc"   ,
    "easeinelastic", "easeoutelastic", "easeinoutelastic", "easeoutinelastic",
    "easeinback"   , "easeoutback"   , "easeinoutback"   , "easeoutinback"   ,
    "easeinbounce" , "easeoutbounce" , "easeinoutbounce" , "easeoutinbounce" ,
};

constexpr auto dirs = { "left", "fromright", "right", "fromleft", "up", "fromdown", "down", "fromtop" };

struct tran
{
    std::string type = "mix";
    msec len = 500ms;
    std::optional<std::string> tween;
    std::optional<std::string> dir;
};

constexpr auto modes = { "original", "stretch", "fit", "fill", "hfill", "vfill" };

struct entry
{
    fs::path path;
    enum { audio, image, video } type;

    std::optional<msec> len;
    std::optional<struct tran> tran;
    std::optional<std::string> mode;
};

struct size { int width, height; };
struct pos  { int x, y; };
struct rate { int num, den; };

struct settings
{
    fs::path media_path;
    fs::path server_path = "casparcg-server";

    struct size size { 1920, 1080 };
    struct pos  pos  { 0, 0 };
    struct rate rate { 60000, 1000 };
    std::string cadence = "800";

    bool full_screen = false;
    bool border = false;
    bool topmost = false;

    msec image_len = 10s;
    bool loop = true;

    std::optional<struct tran> tran;
    std::optional<std::string> mode;
};
