////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2026 Dimitry Ishenko
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
//
// Distributed under the GNU GPL license. See the LICENSE.md file for details.

////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "settings.hpp"
#include "uv/loop.hpp"
#include "uv/process.hpp"

#include <memory>

////////////////////////////////////////////////////////////////////////////////
class server
{
public:
    using exit_callback = uv::process::exit_callback;

    server(uv::loop&, const settings&);
    ~server();

    server(server&&) noexcept = default;
    server& operator=(server&&) noexcept = default;

    void on_exit(exit_callback cb) { *exit_cb_ = std::move(cb); }

private:
    std::unique_ptr<exit_callback> exit_cb_ = std::make_unique<exit_callback>();

    fs::path config_dir_;
    uv::process proc_;
};
