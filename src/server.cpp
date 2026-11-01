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
#include <utility>

#include <uv.h>

////////////////////////////////////////////////////////////////////////////////
server::server(uv::loop& loop, const settings& data) : osc_{loop}
{
    osc_.bind({ "127.0.0.1", 0 });
    osc_.on_recv([&](std::string_view data, const uv::address&)
    {
        osc::packet packet{ data.data(), static_cast<osc::int32>(data.size()) };
        try { space_.dispatch(packet.parse()); }
        catch (...) { } // discard invalid
    });

    space_.add("/channel/1/framerate", [&](auto& msg)
    {
        if (!framerate_) framerate_ = (double)msg.value(0).to_int32() / msg.value(1).to_int32();

        // watchdog for the screen consumer
        if (++watch_ > 10) set_state(stopping);
    });

    space_.add("/channel/1/output/port/.*/consumer", [&](auto& msg)
    {
        // watchdog for the screen consumer
        if (msg.value(0).to_string() == "screen") watch_ = 0;
    });

    space_.add("/channel/1/stage/layer/0/background/producer", [&](auto& msg)
    {
        if (state_ == stopping || !framerate_) return;
        set_state(msg.value(0).to_string() == "empty" ? ready : loaded);
    });

    std::print("[server] generating config\n");

    auto pid = uv_os_getpid();
    auto tmpdir = fs::temp_directory_path() / ("player-" + std::to_string(pid));
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

    std::print("[server] starting CasparCG Server with:\n{}\n", config);

    proc_ = uv::process{loop, { data.server_path, config_path.string() }};
    proc_.on_exit([&](int status, int signal)
    {
        std::print("[server] CasparCG Server exited: status={} signal={}\n", status, signal);
        state_ = stopping;
        if (cb_) cb_(state_);
    });
}

void server::load_next(const entry& entry)
{
    auto path = entry.path.string();
    std::print("[server] loading '{}'\n", path);

    auto cmd = "loadbg 1 \"" + entry.path.string() + "\"";
    if (entry.len)
    {
        int len = entry.len->count() * framerate_ / 1000 + .5;
        proc_.write(std::format("loadbg 1 \"{}\" length {} mix 30 auto\n", path, len));
    }
    else proc_.write(std::format("loadbg 1 \"{}\" mix 30 auto\n", path));
}

void server::set_state(state new_state)
{
    if (new_state != state_)
    {
        state_ = new_state;
        if (cb_) cb_(state_);
    }
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
