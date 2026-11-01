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
#include <osc++.hpp>

////////////////////////////////////////////////////////////////////////////////
class server
{
public:
    server(uv::loop&, const settings&);
    ~server();

    void stop() noexcept { proc_.stop(); }

    enum state { none, started, ready, loaded, stopping };
    using state_change_callback = std::function<void(state)>;
    void on_state_changed(state_change_callback cb) { cb_ = std::move(cb); }

    void load_next(const entry&);

private:
    fs::path config_dir_;

    uv::udp osc_;
    osc::address_space space_;

    uv::process proc_;

    state_change_callback cb_;
    state state_ = none;
    void set_state(state);

    double framerate_ = 0;
    int watch_ = 0;
};
