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
};

struct settings
{
    fs::path media_path;
    fs::path server_path = "casparcg-server";

    struct size { int width = 1920, height = 1080; } size;
    struct pos  { int x = 0, y = 0; } pos;
    struct rate { int num = 60000, den = 1000; } rate;
    std::string cadence = "800";

    bool full_screen = false;
    bool border = false;
    std::string mode = "stretch";
    bool topmost = false;

    msec image_len = 10s;
    bool loop = true;
};
