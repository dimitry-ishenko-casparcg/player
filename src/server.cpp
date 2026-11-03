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
server::tmpdir::tmpdir(int pid) :
    path{ fs::temp_directory_path() / ("player-" + std::to_string(pid)) }
{
    fs::create_directories(path);
    std::print("[server] created temp dir: '{}'\n", path.string());
}

server::tmpdir::~tmpdir()
{
    if (!path.empty())
    {
        std::print("[server] removing temp dir\n");
        std::error_code ec;
        fs::remove_all(path, ec);
    }
}

////////////////////////////////////////////////////////////////////////////////
server::server(uv::loop& loop, const settings& data) :
    tmpdir_{ uv_os_getpid() }, osc_{ loop }
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
        if (++watch_ > 10)
        {
            std::print("[server] stopping\n");
            set_state(stopping);
        }
    });

    space_.add("/channel/1/output/port/.*/consumer", [&](auto& msg)
    {
        // watchdog for the screen consumer
        if (msg.value(0).to_string() == "screen")
        {
            if (state_ == none)
            {
                std::print("[server] starting\n");
                set_state(started);
            }
            watch_ = 0;
        }
    });

    space_.add("/channel/1/stage/layer/0/background/producer", [&](auto& msg)
    {
        if (state_ == stopping) return;
        set_state(msg.value(0).to_string() == "empty" ? ready : loaded);
    });

    std::print("[server] generating config\n");
    std::ofstream file;
    file.exceptions(std::ios_base::failbit | std::ios_base::badbit);

    auto config_path = tmpdir_.path / "casparcg.config";
    file.open(config_path);

    auto mode_id = std::format("{}x{}_{:.0f}",
        data.size.width, data.size.height, 100. * data.rate.num / data.rate.den
    );

    constexpr auto tmpl = R"(<?xml version="1.0" encoding="utf-8"?>
<configuration>
    <paths>
        <media-path>{0}</media-path>
        <log-path disabled="true">{0}</log-path>
        <data-path>{0}</data-path>
        <template-path>{0}</template-path>
    </paths>
    <video-modes>
        <video-mode>
            <id>{1}</id>
            <width>{2}</width><height>{3}</height>
            <time-scale>{4}</time-scale><duration>{5}</duration>
            <cadence>{6}</cadence>
        </video-mode>
    </video-modes>
    <html>
        <cache-path>{0}/cef</cache-path>
    </html>
    <channels>
        <channel>
            <video-mode>{1}</video-mode>
            <consumers>
                <screen>
                    <windowed>{7}</windowed>
                    <borderless>{8}</borderless>
                    <always-on-top>{9}</always-on-top>
                    <x>{10}</x><y>{11}</y>
                </screen>
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
          <port>{12}</port>
        </predefined-client>
      </predefined-clients>
    </osc>
</configuration>
)";
    auto config = std::format(tmpl,
        tmpdir_.path.string(), // 0
        mode_id, data.size.width, data.size.height, data.rate.num, data.rate.den, data.cadence, // 1-6
        !data.full_screen, !data.border, data.topmost, // 7-9
        data.pos.x, data.pos.y, // 10-11
        osc_.local().port() // 12
    );

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
        cmd.append(" length " + std::to_string(len));
    }
    cmd.append(" mix 30");
    if (entry.mode) cmd.append(" scale_mode " + entry.mode.value());

    cmd.append(" auto\n");
    proc_.write(std::move(cmd));
}

void server::set_state(state new_state)
{
    if (new_state != state_)
    {
        state_ = new_state;
        if (cb_) cb_(state_);
    }
}
