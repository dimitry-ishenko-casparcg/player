////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <chrono>
#include <filesystem>

namespace fs = std::filesystem;
using msec = std::chrono::milliseconds;

using namespace std::chrono_literals;

////////////////////////////////////////////////////////////////////////////////
struct settings
{
    fs::path media_path;
    fs::path server_path = "casparcg-server";

    msec image_len = 10s;
    bool loop = true;
};
