////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "settings.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <vector>

namespace fs = std::filesystem;
using msec = std::chrono::milliseconds;

////////////////////////////////////////////////////////////////////////////////
class player
{
    struct entry
    {
        fs::path path;
        enum { audio, image, video } type;
        std::optional<msec> duration;
    };

    std::vector<entry> media_;

public:
    explicit player(const settings&);

    void run();
};
