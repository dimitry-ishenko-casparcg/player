////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#include "server.hpp"
#include "uv/address.hpp"

#include <format>
#include <fstream>
#include <print>
#include <random>
#include <utility>

////////////////////////////////////////////////////////////////////////////////
server::server(uv::loop& loop, const settings& data) : osc_{loop}
{
    osc_.bind({ "127.0.0.1", 0 });
    osc_.on_recv([this](std::string_view data, const uv::address&)
    {
        osc::packet packet{ data.data(), static_cast<osc::int32>(data.size()) };

        try { space_.dispatch(packet.parse()); }
        catch (...) { } // discard invalid
    });

    std::print("[server] generating config\n");

    std::random_device rd;
    std::mt19937_64 gen{rd()};

    auto tmpdir = fs::temp_directory_path() / ("player-" + std::to_string(gen()));
    fs::create_directories(tmpdir);

    std::print("[server] created temp dir: '{}'\n", tmpdir.string());

    config_dir_ = std::move(tmpdir);
    auto config_path = config_dir_ / "casparcg.config";

    std::ofstream file;
    file.exceptions(std::ios_base::failbit | std::ios_base::badbit);
    file.open(config_path);

    constexpr auto tmpl = R"(<?xml version="1.0" encoding="utf-8"?>
<configuration>
    <paths>
        <media-path>{0}</media-path>
        <log-path disabled="true">{0}</log-path>
        <data-path>{0}</data-path>
        <template-path>{0}</template-path>
    </paths>
    <html>
        <cache-path>{0}/cef</cache-path>
    </html>
    <channels>
        <channel>
            <video-mode>1080p5994</video-mode>
            <consumers>
                <screen />
                <system-audio />
            </consumers>
        </channel>
    </channels>
    <controllers>
        <tcp>
            <port>0</port>
            <protocol>amcp</protocol>
        </tcp>
    </controllers>
    <osc>
      <predefined-clients>
        <predefined-client>
          <address>127.0.0.1</address>
          <port>{1}</port>
        </predefined-client>
      </predefined-clients>
    </osc>
</configuration>
)";
    auto config = std::format(tmpl, config_dir_.string(), osc_.local().port());

    file.write(config.data(), config.size());
    file.close();

    std::print("[server] running with config:\n{}\n", config);

    proc_ = uv::process{loop, { data.server_path, config_path.string() }};
    proc_.on_exit([this](int status, int signal){ if (exit_cb_) exit_cb_(status, signal); });
}

server::~server()
{
    if (!config_dir_.empty())
    {
        std::print("[server] removing temp dir\n");
        std::error_code ec;
        fs::remove_all(config_dir_, ec);
    }
}
