////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <chrono>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;
using msec = std::chrono::milliseconds;

using namespace std::chrono_literals;

////////////////////////////////////////////////////////////////////////////////
struct entry
{
    fs::path path;
    enum { audio, image, video } type;

    std::optional<msec> len;
    std::optional<std::string> mode;
};

struct size { int width, height; };
struct pos  { int x, y; };
struct rate { int num, den; };

constexpr auto modes = { "original", "stretch", "fit", "fill", "hfill", "vfill" };

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
    std::string mode = "stretch";
    bool topmost = false;

    msec image_len = 10s;
    bool loop = true;
};
