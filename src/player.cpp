////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "player.hpp"
#include "server.hpp"
#include "uv/loop.hpp"
#include "uv/signal.hpp"

#include <algorithm> // std::ranges::lexicographical_compare
#include <cctype> // std::tolower
#include <print>
#include <set>
#include <string>
#include <string_view>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
namespace
{

struct icase_less
{
    using is_transparent = void;
    bool operator()(auto&& lhs, auto&& rhs) const
    {
        return std::ranges::lexicographical_compare(lhs, rhs,
            [](char x, char y){ return std::tolower((unsigned char)x) < std::tolower((unsigned char)y); }
        );
    }
};

const std::set<std::string, icase_less> audio_exten
{
    ".aac", ".alac", ".flac", ".m4a", ".m4b", ".mp2", ".mp3",
    ".oga", ".ogg", ".opus", ".wav", ".wma",
};

const std::set<std::string, icase_less> image_exten
{
    ".avif", ".bmp", ".gif", ".jpeg", ".jpg", ".jxl", ".png",
    ".tga", ".tif", ".tiff", ".webp",
};

const std::set<std::string, icase_less> video_exten
{
    ".avi", ".flv", ".m2ts", ".m4v", ".mkv", ".mov", ".mp4", ".mpeg",
    ".mpg", ".mts", ".ogv", ".ts", ".vob", ".webm", ".wmv",
};

}

////////////////////////////////////////////////////////////////////////////////
player::player(settings data) : data_{std::move(data)}
{
    auto add_media = [&](const fs::path& path, bool verbose)
    {
        if (fs::is_regular_file(path))
        {
            entry entry;
            auto exten = path.extension().string();
            std::string_view type;

            if (audio_exten.contains(exten))
            {
                entry.type = entry::audio;
                type = "audio";
            }
            else if (image_exten.contains(exten))
            {
                entry.type = entry::image;
                type = "image";
                entry.len = data_.image_len;
            }
            else if (video_exten.contains(exten))
            {
                entry.type = entry::video;
                type = "video";
            }
            else return;

            entry.path = fs::canonical(path);
            std::print("[player] adding {} '{}'\n", type, entry.path.string());

            media_.push_back(std::move(entry));
        }
        else if (verbose) std::print("[player] ignoring '{}'\n", path.string());
    };

    if (fs::is_directory(data_.media_path))
    {
        fs::recursive_directory_iterator iter{ data_.media_path,
            fs::directory_options::follow_directory_symlink | fs::directory_options::skip_permission_denied
        };
        for (auto&& entry : iter) add_media(entry.path(), false);
    }
    else add_media(data_.media_path, true);

    if (media_.empty()) throw std::invalid_argument{"[player] nothing to play!"};

    std::ranges::sort(media_, {}, &entry::path);
}

void player::run()
{
    uv::loop loop;
    server server{loop, data_};

    uv::signals sigset{loop, SIGINT, SIGTERM};
    sigset.on_signal([&](int num){ server.stop(); });

    server.on_state_changed([&](server::state state)
    {
        switch (state)
        {
        case server::ready:
            // TODO
            break;

        case server::stopping:
            loop.stop();
            break;

        default:;
        }
    });

    loop.run();
}
