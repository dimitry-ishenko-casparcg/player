////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "types.hpp"
#include "uv/loop.hpp"
#include "uv/process.hpp"
#include "uv/udp.hpp"

#include <functional>
#include <optional>
#include <osc++.hpp>

////////////////////////////////////////////////////////////////////////////////
class server
{
public:
    server(uv::loop&, const settings&);
    ~server();

    void stop() noexcept { proc_.terminate(); }

    enum state { starting, ready, loaded, stopping };
    using state_change_callback = std::function<void(state)>;
    void on_state_changed(state_change_callback cb) { cb_ = std::move(cb); }

private:
    fs::path config_dir_;

    uv::udp osc_;
    osc::address_space space_;

    uv::process proc_;

    state_change_callback cb_;
    state state_ = starting;
    void set_state(state);

    std::optional<double> fps_;
};
